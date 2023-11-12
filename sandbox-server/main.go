package main

import (
	"context"
	"fmt"
	"net/http"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
)

func main() {
	http.HandleFunc("/", handler)
	http.ListenAndServe(":8080", nil)
}

func handler(w http.ResponseWriter, r *http.Request) {
	user_id := r.URL.Query().Get("user_id")
	algo_id := r.URL.Query().Get("algo_id")
	if user_id == "" || algo_id == ""{
		http.Error(w, "Missing required query parameter", http.StatusBadRequest)
		return
	}

	ctx := context.Background()
	cli, err := client.NewClientWithOpts(client.FromEnv)
	if err != nil {
		panic(err)
	}

	config := &container.Config{
		Image:        "nutc-exchange",
		Cmd:          []string{"--sandbox", user_id, algo_id},
	}
	hostConfig := &container.HostConfig{
		AutoRemove: true,
	}

	resp, err := cli.ContainerCreate(ctx, config, hostConfig, nil, nil, "")
	if err != nil {
		panic(err)
	}

	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		panic(err)
	}

	fmt.Fprintf(w, "Container %s started successfully with user_id: %s and algo_id: %s", resp.ID, user_id, algo_id)
}
