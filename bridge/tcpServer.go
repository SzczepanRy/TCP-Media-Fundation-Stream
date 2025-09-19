package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"net"
	"net/http"
	"sync"

	"github.com/gorilla/websocket"
)

var (
	clients   = make(map[*websocket.Conn]bool)
	clientsMu sync.Mutex
	upgrader  = websocket.Upgrader{
		CheckOrigin: func(r *http.Request) bool { return true },
	}
)

type TCPServer struct{}

func (T TCPServer) Listen() {

	http.HandleFunc("/ws", handleWS)
	go func() {
		log.Printf("WebSocket server listening on ws://localhost:%d", 9000)
    log.Fatal(http.ListenAndServe(":9000", nil))
	}()

	addr := fmt.Sprintf(":%d", 12345)
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		log.Fatalf("Error starting TCP server: %v", err)
	}
	defer ln.Close()

	log.Printf("TCP server listening on 127.0.0.1:%d", 12345)

	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Printf("Error accepting connection: %v", err)
			continue
		}
		log.Println("C++ client connected")
		go handleTCP(conn)
	}
}

func handleWS(w http.ResponseWriter, r *http.Request) {
	ws, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("WebSocket upgrade error: %v", err)
		return
	}

	clientsMu.Lock()
	clients[ws] = true
	clientsMu.Unlock()

	defer func() {
		clientsMu.Lock()
		delete(clients, ws)
		clientsMu.Unlock()
		ws.Close()
	}()

	// Keep open
	for {
		if _, _, err := ws.NextReader(); err != nil {
			break
		}
	}
}

func handleTCP(conn net.Conn) {
	defer func() {
		log.Println("C++ client disconnected")
		conn.Close()
	}()

	buf := make([]byte, 0)
	tmp := make([]byte, 1024)

	for {
		n, err := conn.Read(tmp)
		if err != nil {
			return
		}
		buf = append(buf, tmp[:n]...)

		// Frame parsing loop
		for len(buf) >= 4 {
			frameSize := binary.BigEndian.Uint32(buf[:4])
			if uint32(len(buf)) < 4+frameSize {
				break
			}

			frame := buf[4 : 4+frameSize]
			buf = buf[4+frameSize:]

			broadcast(frame)
		}
	}
}

func broadcast(frame []byte) {
	clientsMu.Lock()
	defer clientsMu.Unlock()

	for ws := range clients {
		if err := ws.WriteMessage(websocket.BinaryMessage, frame); err != nil {
			log.Printf("Error sending to WebSocket client: %v", err)
			ws.Close()
			delete(clients, ws)
		}
	}
}
