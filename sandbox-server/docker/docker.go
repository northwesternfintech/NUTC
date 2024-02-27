package docker

// import (
// 	"context"
// 	"io"
// 	"log"

// 	"github.com/docker/docker/api/types"
// 	"github.com/docker/docker/client"
// )

// // DockerClient wraps the Docker API client with additional functionalities.
// type DockerClient struct {
// 	client *client.Client
// }

// // NewDockerClient creates a new DockerClient instance, encapsulating the initialization of the Docker API client.
// func NewDockerClient() (*DockerClient, error) {
// 	cli, err := client.NewClientWithOpts(client.FromEnv)
// 	if err != nil {
// 		return nil, err
// 	}
// 	return &DockerClient{client: cli}, nil
// }

// // CheckAndPullImage ensures that the specified image is available locally, pulling it if necessary.
// func (dc *DockerClient) CheckAndPullImage(ctx context.Context, imageName string) error {
// 	images, err := dc.client.ImageList(ctx, types.ImageListOptions{})
// 	if err != nil {
// 		return err
// 	}

// 	// Check if image exists locally
// 	for _, image := range images {
// 		for _, tag := range image.RepoTags {
// 			if tag == imageName {
// 				log.Println("Image found locally")
// 				return nil
// 			}
// 		}
// 	}

// 	log.Println("Image not found locally, pulling...")
// 	return dc.pullImage(ctx, imageName)
// }

// // pullImage pulls the specified image using Docker Engine API.
// func (dc *DockerClient) pullImage(ctx context.Context, imageName string) error {
// 	out, err := dc.client.ImagePull(ctx, imageName, types.ImagePullOptions{})
// 	if err != nil {
// 		return err
// 	}
// 	defer out.Close()

// 	_, err = io.Copy(io.Discard, out)
// 	if err != nil {
// 		return err
// 	}

// 	log.Println("Image pulled successfully")
// 	return nil
// }

// // LoginToRegistry logs in to a Docker registry with the specified credentials.
// func (dc *DockerClient) LoginToRegistry(ctx context.Context, username, password, registry string) error {
// 	authConfig := types.AuthConfig{
// 		Username:      username,
// 		Password:      password,
// 		ServerAddress: registry, // Adjust as needed
// 	}
// 	_, err := dc.client.RegistryLogin(ctx, authConfig)
// 	if err != nil {
// 		return err
// 	}

// 	log.Println("Registry login successful")
// 	return nil
// }
