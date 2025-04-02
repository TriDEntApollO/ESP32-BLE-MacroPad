package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strconv"
	"sync"
)

type KeyMap struct {
	Layers       int
	Rows, Cols   int
	MacroMessage string
	Data         [10][3][3]string // 10 layers, 3 rows, 3 cols
	LayerNames   [10]string       // Layer names for 10 layers
	mu           sync.Mutex
}

var keyMap = KeyMap{
	Layers:       3,
	Rows:         3,
	Cols:         3,
	MacroMessage: "Default Macro Message",
	LayerNames:   [10]string{"Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Layer 6", "Layer 7", "Layer 8", "Layer 9", "Layer 10"},
}

func handleRoot(w http.ResponseWriter, r *http.Request) {
	if r.URL.Path != "/" {
		http.NotFound(w, r)
		return
	}
	http.ServeFile(w, r, "test.html")
}

func handleStatus(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
	fmt.Fprintln(w, "connected")
}

func handleGetActiveLayer(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintln(w, keyMap.Layers)
}

func handleChangeActiveLayers(w http.ResponseWriter, r *http.Request) {
	value, err := strconv.Atoi(r.URL.Query().Get("value"))
	if err != nil || value < 1 || value > 10 {
		http.Error(w, "Invalid Layers! Layers out of range", http.StatusBadRequest)
		return
	}
	keyMap.Layers = value
	fmt.Fprintln(w, "Active layers updated to", keyMap.Layers)
}

func handleGetLayerNames(w http.ResponseWriter, r *http.Request) {
	keyMap.mu.Lock()
	defer keyMap.mu.Unlock()
	activeLayerNames := keyMap.LayerNames[:keyMap.Layers]

	w.Header().Set("Content-Type", "application/json")
	if err := json.NewEncoder(w).Encode(activeLayerNames); err != nil {
		http.Error(w, "Failed to encode layer names", http.StatusInternalServerError)
	}
}

func handleUpdateLayerName(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()
	layerIndex, err := strconv.Atoi(r.FormValue("index"))
	if err != nil || layerIndex < 0 || layerIndex >= keyMap.Layers {
		http.Error(w, "Invalid layer index", http.StatusBadRequest)
		return
	}

	newLayerName := r.FormValue("name")
	if newLayerName == "" {
		http.Error(w, "Invalid name: Name cannot be empty", http.StatusBadRequest)
		return
	}

	keyMap.mu.Lock()
	defer keyMap.mu.Unlock()
	keyMap.LayerNames[layerIndex] = newLayerName
	fmt.Fprintf(w, "Layer %d name updated to \"%s\" successfully.", layerIndex, newLayerName)
}

func handleSubmitKeymap(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()
	layer, err := strconv.Atoi(r.FormValue("layer"))
	if err != nil || layer < 0 || layer >= keyMap.Layers {
		http.Error(w, "Invalid Layer", http.StatusBadRequest)
		return
	}

	data := r.FormValue("data")
	if len(data) != keyMap.Rows*keyMap.Cols {
		http.Error(w, "Invalid Data Length", http.StatusBadRequest)
		return
	}

	keyMap.mu.Lock()
	index := 0
	for row := 0; row < keyMap.Rows; row++ {
		for col := 0; col < keyMap.Cols; col++ {
			keyMap.Data[layer][row][col] = string(data[index])
			index++
		}
	}
	keyMap.mu.Unlock()

	fmt.Fprintln(w, "OK")
}

func handleGetKeymap(w http.ResponseWriter, r *http.Request) {
	layer, err := strconv.Atoi(r.URL.Query().Get("layer"))
	if err != nil || layer < 0 || layer >= 10 {
		http.Error(w, "Invalid Layer", http.StatusBadRequest)
		return
	}

	json.NewEncoder(w).Encode(keyMap.Data[layer])
}

func handleGetMacroMessage(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintln(w, keyMap.MacroMessage)
}

func handleUpdateMacroMessage(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()
	message := r.FormValue("message")
	if message == "" {
		http.Error(w, "Error: No message received", http.StatusBadRequest)
		return
	}

	keyMap.MacroMessage = message
	fmt.Fprintln(w, "Macro message updated successfully.")
}

func handleNotFound(w http.ResponseWriter, r *http.Request) {
	http.Error(w, "Error 404: Page Not Found", http.StatusNotFound)
}

func main() {
	http.HandleFunc("/", handleRoot)
	http.HandleFunc("/status", handleStatus)
	http.HandleFunc("/getActiveLayers", handleGetActiveLayer)
	http.HandleFunc("/changeActiveLayers", handleChangeActiveLayers)
	http.HandleFunc("/submitKeymap", handleSubmitKeymap)
	http.HandleFunc("/getKeymap", handleGetKeymap)
	http.HandleFunc("/getMacroMessage", handleGetMacroMessage)
	http.HandleFunc("/updateMacroMessage", handleUpdateMacroMessage)
	http.HandleFunc("/getLayerNames", handleGetLayerNames)
	http.HandleFunc("/updateLayerName", handleUpdateLayerName)
	http.NotFoundHandler()

	fmt.Println("Starting CronoDeck Web Server on port 8080...")
	http.ListenAndServe(":8080", nil)
}
