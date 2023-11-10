package main

import (
	"fmt"
	"io"
	"log"
	"net/http"

	"github.com/gorilla/mux"
)

const firebaseURL = "https://nutc-web-default-rtdb.firebaseio.com/"

func main() {
	r := mux.NewRouter()
	r.HandleFunc("/", handler)
	log.Fatal(http.ListenAndServe(":8080", r))

}

func handler(w http.ResponseWriter, r *http.Request) {
	userID := r.URL.Query().Get("user_id")
	algoID := r.URL.Query().Get("algo_id")

	if userID == "" || algoID == "" {
		http.Error(w, "Missing parameter", http.StatusBadRequest)
		return
	}

	fbData, err := getFirebaseData(userID, algoID)
	if err != nil {
		http.Error(w, "Firebase error", http.StatusInternalServerError)
		return
	}

	err = spawnDockerContainer(userID, algoID)
	if err != nil {
		http.Error(w, "Docker error", http.StatusInternalServerError)
		return
	}

	w.Write([]byte(fmt.Sprintf("Container started for User ID: %s, Algo ID: %s, Firebase Data: %s", userID, algoID, fbData)))
}

func getFirebaseData(userID string, algoID string) (string, error) {
	resp, err := http.Get(fmt.Sprintf("%s/users/%s/algos/%s.json", firebaseURL, userID, algoID))
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}

	return string(body), nil
}

func spawnDockerContainer(userID string, algoID string) error {
	fmt.Print("Spawning container,...")
	return nil
	// cmd := exec.Command("docker", "run", "--sandbox", userID, algoID, "docker_image")
	// return cmd.Run()
}
