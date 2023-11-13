package main

import (
	"fmt"
	"io"
	"net/http"
	"time"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	gonanoid "github.com/matoous/go-nanoid/v2"
)

const firebaseURL = "https://nutc-web-default-rtdb.firebaseio.com/"

func main() {
	server := http.Server{
		Addr:         ":8080",
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 10 * time.Second,
		IdleTimeout:  15 * time.Second,
	}
	http.HandleFunc("/", handler)

	if err := server.ListenAndServe(); err != nil {
		fmt.Printf("Failed to start server\n")
	}
}

func handler(w http.ResponseWriter, r *http.Request) {
	user_id := r.URL.Query().Get("user_id")
	algo_id := r.URL.Query().Get("algo_id")
	if user_id == "" || algo_id == "" {
		http.Error(w, "Missing required query parameter", http.StatusBadRequest)
		return
	}

	if !isValidUserAndAlgoID(user_id, algo_id) {
		http.Error(w, "Invalid user or algo ID", http.StatusBadRequest)
		return
	}

	ctx := r.Context()
	cli, err := client.NewClientWithOpts(client.FromEnv)
	if err != nil {
		http.Error(w, "Failed to initialize docker client", http.StatusInternalServerError)
		return
	}

	config := &container.Config{
		Image: "nutc-exchange",
		// What happens if user or algo id start with "-" since it breaks arg parsing?
		Cmd: []string{"--sandbox", user_id, algo_id},
	}
	hostConfig := &container.HostConfig{
		AutoRemove: true,
	}

	nano_id, err := gonanoid.New(6)
	if err != nil {
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		return
	}
	container_name := fmt.Sprintf("%s-%s-%s", user_id, algo_id, nano_id)
	resp, err := cli.ContainerCreate(ctx, config, hostConfig, nil, nil, container_name)
	if err != nil {
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		return
	}

	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		http.Error(w, "Failed to start docker container", http.StatusInternalServerError)
		return
	}

	fmt.Fprintf(w, "Container %s started successfully with user_id: %s and algo_id: %s\n", resp.ID, user_id, algo_id)
}

func isValidUserAndAlgoID(userID string, algoID string) bool {
	resp, err := http.Get(fmt.Sprintf("%s/users/%s/algos/%s.json", firebaseURL, userID, algoID))
	if err != nil {
		return false
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return false
	}

	// If the user and algo id don't exist, Firebase returns "null"
	return string(body) != "null"
}
