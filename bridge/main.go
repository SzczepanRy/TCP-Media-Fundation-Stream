package main

import (
	"fmt"
	"net/http"
)

func main() {

	go func() {
		R := Router{}
		fmt.Println("4000")
		http.ListenAndServe(":4000", &R)
	}()

	T := TCPServer{}
  T.Listen()

}
