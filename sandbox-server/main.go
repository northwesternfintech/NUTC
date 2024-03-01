package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"mime/multipart"
	"net/http"
	"net/url"
	"sandbox-server/analyzer"
	"strings"
	"time"
	"unicode"

	"archive/tar"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	gonanoid "github.com/matoous/go-nanoid/v2"
)

const dockerTimeout = time.Minute * 1
const firebaseStorageUrl = "https://firebasestorage.googleapis.com/v0/b/nutc-web.appspot.com/o"
const firebaseApiKey = "AIzaSyCo2l3x2DMhg5CaNy1Pyvknk_GK8v34iUc"

const port = "12687"

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
		log.Printf("ERROR: Failed to initialize docker client: %s", err.Error())
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
		AutoRemove: false,
	}

	nano_id, err := gonanoid.New(6)
	if err != nil {
		log.Printf("ERROR: Failed to generate nano id: %s", err.Error())
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		fmt.Printf("%s", err.Error())
		return
	}

	// container name cannot contain spaces and cannot start with "-"
	container_name := fmt.Sprintf("%s-%s-%s", strings.TrimSpace(cmd_user_id), strings.TrimSpace(cmd_algo_id), nano_id)
	log.Printf("Starting container with name: %s\n", container_name)
	resp, err := cli.ContainerCreate(ctx, config, hostConfig, nil, nil, container_name)
	if err != nil {
		log.Printf("ERROR: Failed to create docker container: %s\n", err.Error())
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		fmt.Printf("%s", err.Error())
		return
	}

	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		log.Printf("ERROR: Failed to start docker container: %s\n", err.Error())
		http.Error(w, "Failed to start docker container", http.StatusInternalServerError)
		return
	}

	go func() {
		defer cli.ContainerStop(context.Background(), resp.ID, container.StopOptions{})

		time.Sleep(dockerTimeout)

		reader, _, err := cli.CopyFromContainer(context.Background(), resp.ID, "logs/structured.log")
		if err != nil {
			log.Printf("ERROR: Failed to copy log file from container: %s\n", err.Error())
			fmt.Printf("%s", err.Error())
			return
		}
		defer reader.Close()

		tarReader := tar.NewReader(reader)

		_, err = tarReader.Next()
		if err != nil {
			fmt.Printf("%s", err.Error())
			return
		}

		out_file, err := analyzer.Analyze(tarReader, user_id)

		if err != nil {
			log.Printf("ERROR: Failed to analyze log file: %s\n", err.Error())
			fmt.Printf("%s", err.Error())
		}

		download_token, err := uploadLogFile(user_id, algo_id, firebaseApiKey, out_file)
		if err != nil {
			log.Printf("ERROR: Failed to upload log file: %s\n", err.Error())
			fmt.Printf("%s", err.Error())
		}

		file_url := fmt.Sprintf("%s/%s?alt=media&token=%s", firebaseStorageUrl, url.PathEscape("logs/"+user_id+"/"+algo_id+".log"), download_token)

		err = addLogFileUrlToUser(user_id, algo_id, file_url)
		if err != nil {
			log.Printf("ERROR: Failed to add log file url to user algo: %s\n", err.Error())
			fmt.Printf("%s", err.Error())
		}
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

func uploadLogFile(user_id, algo_id, apiKey, file_str string) (string, error) {
	reader := strings.NewReader(file_str)
	var buffer bytes.Buffer

	writer := multipart.NewWriter(&buffer)

	fileName := fmt.Sprintf("logs/%s/%s.log", user_id, algo_id)

	part, err := writer.CreateFormFile("file", fileName)
	if err != nil {
		return "", fmt.Errorf("writer.CreateFormFile: %v", err)
	}

	if _, err := io.Copy(part, reader); err != nil {
		return "", fmt.Errorf("io.Copy: %v", err)
	}

	if err := writer.Close(); err != nil {
		return "", fmt.Errorf("writer.Close: %v", err)
	}

	req, err := http.NewRequest("POST", firebaseStorageUrl+"?uploadType=media&name="+fileName, &buffer)
	if err != nil {
		return "", fmt.Errorf("http.NewRequest: %v", err)
	}

	req.Header.Set("Authorization", "Bearer "+apiKey)
	req.Header.Set("Content-Type", writer.FormDataContentType())

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		log.Printf("ERROR: Client.do: Failed to upload log file: %s", err.Error())
		return "", fmt.Errorf("client.Do: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		log.Printf("ERROR: Failed to upload log file: %s", resp.Status)
		return "", fmt.Errorf("upload failed with status: %v", resp.Status)
	}

	bodyBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("ioutil.ReadAll: %v", err)
	}

	// Unmarshal the JSON response
	var jsonResponse map[string]interface{}
	if err := json.Unmarshal(bodyBytes, &jsonResponse); err != nil {
		return "", fmt.Errorf("json.Unmarshal: %v", err)
	}

	downloadToken, ok := jsonResponse["downloadTokens"].(string)
	if !ok {
		log.Printf("ERROR: download token not found in response: %v", jsonResponse)
		return "", fmt.Errorf("download token not found in response")
	}

	return downloadToken, nil
}

// Add log file url to user algo in firebase database
func addLogFileUrlToUser(user_id, algo_id, file_url string) error {
	type UserData struct {
		SandboxLogFileUrl string `json:"sandboxLogFileURL"`
	}

	data := UserData{file_url}

	jsonData, err := json.Marshal(data)
	if err != nil {
		return fmt.Errorf("json.Marshal: %v", err)
	}

	req, err := http.NewRequest("PATCH", "https://nutc-web-default-rtdb.firebaseio.com/users/"+user_id+"/algos/"+algo_id+".json", bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("http.NewRequest: %v", err)
	}

	req.Header.Set("Content-Type", "application/json")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		log.Printf("ERROR: Client.do: Failed to add log file url to user algo: %s", err.Error())
		return fmt.Errorf("client.Do: %v", err)
	}

	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		log.Printf("ERROR: Failed to add log file url to user algo: %s", resp.Status)
		return fmt.Errorf("upload failed with status: %v", resp.Status)
	}

	return nil
}
