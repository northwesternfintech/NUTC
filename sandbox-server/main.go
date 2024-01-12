package main

import (
	"context"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strings"
	"time"
	"unicode"

	"archive/tar"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	gonanoid "github.com/matoous/go-nanoid/v2"
)

const dockerTimeout = time.Minute * 10

const port = "8081"

func main() {
	server := http.Server{
		Addr:         ":" + port,
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 10 * time.Second,
		IdleTimeout:  15 * time.Second,
	}
	http.HandleFunc("/", algoTestingHandler)

	log.Printf("Sandbox server listening on port %s\n", port)
	if err := server.ListenAndServe(); err != nil {
		log.Fatal("Failed to start server\n")
	}
}

func algoTestingHandler(w http.ResponseWriter, r *http.Request) {
	user_id := r.URL.Query().Get("user_id")
	algo_id := r.URL.Query().Get("algo_id")
	if user_id == "" || algo_id == "" {
		http.Error(w, "Missing required query parameter", http.StatusBadRequest)
		return
	}

	if !isValidID(user_id) || !isValidID(algo_id) {
		http.Error(w, "Invalid user or algo ID", http.StatusBadRequest)
		return
	}

	ctx := r.Context()
	cli, err := client.NewClientWithOpts(client.FromEnv)
	if err != nil {
		http.Error(w, "Failed to initialize docker client", http.StatusInternalServerError)
		return
	}

	cmd_user_id := user_id
	cmd_algo_id := algo_id
	if strings.HasPrefix(user_id, "-") {
		cmd_user_id = " " + user_id[1:]
	}
	if strings.HasPrefix(algo_id, "-") {
		cmd_algo_id = " " + algo_id[1:]
	}

	config := &container.Config{
		Image: "nutc-exchange",
		Cmd:   []string{"--sandbox", cmd_user_id, cmd_algo_id},
	}
	hostConfig := &container.HostConfig{
		AutoRemove: true,
	}

	nano_id, err := gonanoid.New(6)
	if err != nil {
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		fmt.Printf("%s", err.Error())
		return
	}

	// container name cannot contain spaces and cannot start with "-"
	container_name := fmt.Sprintf("%s-%s-%s", strings.TrimSpace(cmd_user_id), strings.TrimSpace(cmd_algo_id), nano_id)
	resp, err := cli.ContainerCreate(ctx, config, hostConfig, nil, nil, container_name)
	if err != nil {
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		fmt.Printf("%s", err.Error())
		return
	}

	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		http.Error(w, "Failed to start docker container", http.StatusInternalServerError)
		return
	}

	go func() {
		time.Sleep(dockerTimeout)
		reader, _, err := cli.CopyFromContainer(context.Background(), resp.ID, "logs/app.log")
		if err != nil {
			fmt.Printf("%s", err.Error())
		}
		defer reader.Close()

		tarReader := tar.NewReader(reader)

		_, err = tarReader.Next()
		if err != nil {
			fmt.Printf("%s", err.Error())
		}

		dstFile, err := os.Create("out")
		if err != nil {
			fmt.Printf("%s", err.Error())
		}
		defer dstFile.Close()

		_, err = io.Copy(dstFile, tarReader)
		if err != nil {
			fmt.Printf("%s", err.Error())
		}

		cli.ContainerStop(context.Background(), resp.ID, container.StopOptions{})
	}()

	fmt.Fprintf(w, "Container %s started successfully with user_id: %s and algo_id: %s\n", container_name, user_id, algo_id)
}

func isValidID(id string) bool {
	for _, char := range id {
		if !unicode.IsLetter(char) && !unicode.IsDigit(char) && char != '-' && char != '_' {
			return false
		}
	}
	return true
}
