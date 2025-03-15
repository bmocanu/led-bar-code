package main

import (
	_ "monitoring-gateway/config"
	"monitoring-gateway/server"
)

func main() {
	// config.init() 	=> sets up the configuration based on env vars
	// retrieve.init() 	=> sets up the cron tab and schedules the periodical comic scanning
	// store.init() 	=> sets up the issue storage
	// serve.init()		=> sets up the HTTP interface + controllers

	// start listening for incoming HTTP requests
	serve.StartServing()
}
