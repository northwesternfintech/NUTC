package main

import (
	"archive/tar"
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"net/url"
	"os"
	"sandbox-server/analyzer"
	"strings"
	"time"
	"unicode"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	gonanoid "github.com/matoous/go-nanoid/v2"
	"go.uber.org/zap"
)

const dockerTimeout = time.Minute * 1
const firebaseStorageUrl = "https://firebasestorage.googleapis.com/v0/b/nutc-web.appspot.com/o"
const firebaseApiKey = "AIzaSyCo2l3x2DMhg5CaNy1Pyvknk_GK8v34iUc"

func enableCors(w *http.ResponseWriter) {
	(*w).Header().Set("Access-Control-Allow-Origin", "*")
}

func initZapLogger() *zap.SugaredLogger {
	logger, _ := zap.NewDevelopment()
	zap.ReplaceGlobals(logger)
	return logger.Sugar()
}

func main() {
	sugar := initZapLogger()
	defer sugar.Sync()

	var port = os.Getenv("SANDBOX_SERVER_PORT")
	if port == "" {
		port = "12687"
	}
	server := http.Server{
		Addr:         ":" + port,
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 10 * time.Second,
		IdleTimeout:  15 * time.Second,
	}
	http.HandleFunc("/", algoTestingHandler)

	sugar.Infof("Sandbox server listening on port %s", port)
	if err := server.ListenAndServe(); err != nil {
		sugar.Fatalf("Failed to start server: %v", err)
	}
}

func algoTestingHandler(w http.ResponseWriter, r *http.Request) {
	sugar := zap.L().Sugar()
	sugar.Infof("Request received from %s", r.RemoteAddr)
	enableCors(&w)
	user_id := r.URL.Query().Get("user_id")
	algo_id := r.URL.Query().Get("algo_id")
	if user_id == "" || algo_id == "" {
		sugar.Warn("Missing required query parameter")
		http.Error(w, "Missing required query parameter", http.StatusBadRequest)
		return
	}

	if !isValidID(user_id) || !isValidID(algo_id) {
		sugar.Warn("Invalid user or algo ID")
		http.Error(w, "Invalid user or algo ID", http.StatusBadRequest)
		return
	}

	ctx := r.Context()
	cli, err := client.NewClientWithOpts(client.FromEnv)
	if err != nil {
		sugar.Errorf("Failed to initialize docker client: %v", err)
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

	imageName := "nutc-exchange"
	exchangeCmd := []string{"--sandbox", cmd_user_id, cmd_algo_id}

	config := &container.Config{
		Image: imageName,
		Cmd:   exchangeCmd,
	}
	
	hostConfig := &container.HostConfig{
		AutoRemove: false,
	}

	nanoID, err := gonanoid.New(6)
	if err != nil {
		sugar.Errorf("Failed to generate nanoID: %v", err)
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		return
	}

	containerName := fmt.Sprintf("%s-%s-%s", strings.TrimSpace(user_id), strings.TrimSpace(algo_id), nanoID)

	sugar.Infof("Starting container with name %s", containerName)
	sugar.Infof("Container cmd: %s", exchangeCmd)
	resp, err := cli.ContainerCreate(ctx, config, hostConfig, nil, nil, containerName)
	if err != nil {
		sugar.Errorf("Failed to create docker container: %v", err)
		http.Error(w, "Failed to create docker container", http.StatusInternalServerError)
		return
	}

	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		sugar.Errorf("Failed to start docker container: %v", err)
		http.Error(w, "Failed to start docker container", http.StatusInternalServerError)
		return
	}

	// wait 10 seconds and check if the container is still running
	time.Sleep(10 * time.Second)
	inspect, err := cli.ContainerInspect(ctx, resp.ID)
	if err != nil {
		sugar.Errorf("Failed to inspect docker container: %v", err)
		http.Error(w, "Failed to inspect docker container", http.StatusInternalServerError)
		return
	}

	if inspect.State.Running {
		sugar.Infof("Container %s is still running", containerName)
	} else {
		sugar.Infof("Container %s has stopped", containerName)
		http.Error(w, "Container failed to start", http.StatusInternalServerError)
		return
	}

	go func() {
		// Ensure to handle errors within this goroutine using the sugar logger
		defer cli.ContainerStop(context.Background(), resp.ID, container.StopOptions{})

		time.Sleep(dockerTimeout)

		reader, _, err := cli.CopyFromContainer(context.Background(), resp.ID, "logs/structured.log")
		if err != nil {
			sugar.Errorf("Failed to copy from container: %v", err)
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
			fmt.Printf("%s", err.Error())
		}

		download_token, err := uploadLogFile(user_id, algo_id, firebaseApiKey, out_file)
		if err != nil {
			fmt.Printf("%s", err.Error())
		}

		file_url := fmt.Sprintf("%s/%s?alt=media&token=%s", firebaseStorageUrl, url.PathEscape("logs/"+user_id+"/"+algo_id+".log"), download_token)

		err = addLogFileUrlToUser(user_id, algo_id, file_url)
		if err != nil {
			fmt.Printf("%s", err.Error())
		}

		sugar.Infof("Log file uploaded successfully: %s", file_url)
	}()

	fmt.Fprintf(w, "Container %s started successfully with user_id: %s and algo_id: %s\n", containerName, user_id, algo_id)
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
	sugarNoContext := zap.L().Sugar()
	sugar := sugarNoContext.With(
		"user_id", user_id,
		"algo_id", algo_id,
		"file_str", file_str,
	)
	sugar.Infoln("Uploading log file")
	reader := strings.NewReader(file_str)
	var buffer bytes.Buffer

	writer := multipart.NewWriter(&buffer)

	fileName := fmt.Sprintf("logs/%s/%s.log", user_id, algo_id)

	part, err := writer.CreateFormFile("file", fileName)
	if err != nil {
		sugar.Errorf("writer.CreateFormFile: %v", err)
		return "", fmt.Errorf("writer.CreateFormFile: %v", err)
	}

	if _, err := io.Copy(part, reader); err != nil {
		sugar.Errorf("io.Copy: %v", err)
		return "", fmt.Errorf("io.Copy: %v", err)
	}

	if err := writer.Close(); err != nil {
		sugar.Errorf("writer.Close: %v", err)
		return "", fmt.Errorf("writer.Close: %v", err)
	}

	req, err := http.NewRequest("POST", firebaseStorageUrl+"?uploadType=media&name="+fileName, &buffer)
	if err != nil {
		sugar.Errorf("http.NewRequest: %v", err)
		return "", fmt.Errorf("http.NewRequest: %v", err)
	}

	req.Header.Set("Authorization", "Bearer "+apiKey)
	req.Header.Set("Content-Type", writer.FormDataContentType())

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		sugar.Errorf("client.Do: %v", err)
		return "", fmt.Errorf("client.Do: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		sugar.Errorf("upload failed with status: %v", resp.Status)
		return "", fmt.Errorf("upload failed with status: %v", resp.Status)
	}

	bodyBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("ioutil.ReadAll: %v", err)
	}

	// Unmarshal the JSON response
	var jsonResponse map[string]interface{}
	if err := json.Unmarshal(bodyBytes, &jsonResponse); err != nil {
		sugar.Errorf("json.Unmarshal: %v", err)
		return "", fmt.Errorf("json.Unmarshal: %v", err)
	}

	downloadToken, ok := jsonResponse["downloadTokens"].(string)
	if !ok {
		sugar.Errorf("download token not found in response")
		return "", fmt.Errorf("download token not found in response")
	}

	return downloadToken, nil
}

// Add log file url to user algo in firebase database
func addLogFileUrlToUser(user_id, algo_id, file_url string) error {
	sugarNoContext := zap.L().Sugar()
	sugar := sugarNoContext.With(
		"user_id", user_id,
		"algo_id", algo_id,
		"file_url", file_url,
	)
	sugar.Infoln("Uploading log file")

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
		return fmt.Errorf("client.Do: %v", err)
	}

	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("upload failed with status: %v", resp.Status)
	}

	return nil
}
