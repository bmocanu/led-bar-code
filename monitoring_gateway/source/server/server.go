package serve

import (
	_ "github.com/gorilla/feeds"
	"github.com/gorilla/mux"
	log "github.com/sirupsen/logrus"
	"monitoring-gateway/config"
	"net/http"
	"strconv"
)

var httpHandler http.Handler

func init() {
	var contextPath = config.Server.ContextPath
	var localHandler = mux.NewRouter()
	//localHandler.HandleFunc(urlConcat(contextPath, ""), getFeedList).Methods("GET")
	localHandler.HandleFunc(urlConcat(contextPath, "/monitoring-csv"), getMonitoringCsv).Methods("GET")
	httpHandler = localHandler
}

func StartServing() {
	var addressAndPort = config.Server.ListenAddress + ":" + strconv.Itoa(config.Server.ListenPort)
	log.Info("Listening for HTTP requests on: ", addressAndPort, ": contextPath: ", config.Server.ContextPath)
	log.Fatal(http.ListenAndServe(addressAndPort, httpHandler))
}
