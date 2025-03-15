package config

import (
	log "github.com/sirupsen/logrus"
	"os"
)

func fileExists(file string) bool {
	if _, err := os.Stat(file); !os.IsNotExist(err) {
		return true
	} else {
		return false
	}
}

func logAppBanner() {
	log.Info("--------------------------------------------------")
	log.Info("LED Bar Monitoring Gateway | ver. ", AppVersion)
	log.Info("--------------------------------------------------")
}

func logConfigState() {
	log.Info("Config log: ListenAddress=", Server.ListenAddress)
	log.Info("Config log: ListenPort=", Server.ListenPort)
	log.Info("Config log: ContextPath=", Server.ContextPath)
	log.Info("Config log: BaseUrl=", Server.BaseUrl)
}
