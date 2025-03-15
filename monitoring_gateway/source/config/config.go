package config

import (
	log "github.com/sirupsen/logrus"
	"monitoring-gateway/model"
	"os"
)

const AppVersion = "1.0"
const AppReleaseDate = "Mar/15/2025"

var Server model.ServerConfig

var appDir string
var configDir string
var configFile string

func init() {
	log.SetLevel(log.InfoLevel)
	log.SetFormatter(&log.TextFormatter{FullTimestamp: true})
	logAppBanner()

	parseCommandLineArgs()

	if !fileExists(configFile) {
		log.Fatal("Cannot find configuration file: ", configFile)
		os.Exit(model.ExitConfigFileNotFound)
	}

	var err = loadConfigFromFile(configFile)
	if err != nil {
		log.Fatal("Cannot load configuration file: ", configFile, ": ", err)
		os.Exit(model.ExitConfigFileNotLoaded)
	}

	logConfigState()
}
