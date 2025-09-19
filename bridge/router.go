package main

import (
	"fmt"
	"net/http"
	"regexp"
	"strings"
)

type Router struct{}

func (R *Router) ServeHTTP(w http.ResponseWriter, r *http.Request) {

	var mimeTypes = map[string]string{"js": "application/javascript; charset=utf-8", "css": "text/css; charset=utf-8"}

	switch r.Method {
	case "GET":
		fmt.Println(r.URL.Path)
		if r.URL.Path == "/" {
			http.ServeFile(w, r, "./fe/index.html")
		}
		if match, _ := regexp.MatchString("/fe/.*", r.URL.Path); match {
			val := strings.Split(r.URL.Path, ".")
			w.Header().Set("Content-Type", mimeTypes[val[1]])
			fmt.Println(mimeTypes[val[1]])
			http.ServeFile(w, r, "./"+r.URL.Path)
		}

	}

}
