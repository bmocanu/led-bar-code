package config

import (
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"os"
)

type jsonModel struct {
	Server struct {
		Address     string `json:"address"`
		Port        int    `json:"port"`
		ContextPath string `json:"contextPath"`
		BaseUrl     string `json:"baseUrl"`
	} `json:"server"`
}

func loadConfigFromFile(configFileStr string) error {
	log.Info("Loading config from file: ", configFileStr)
	var config jsonModel

	configFile, err := os.Open(configFileStr)
	if err != nil {
		return err
	}

	defer configFile.Close()

	jsonParser := json.NewDecoder(configFile)
	err = jsonParser.Decode(&config)
	if err != nil {
		return err
	}

	Server.ListenAddress = config.Server.Address
	Server.ListenPort = config.Server.Port
	Server.ContextPath = config.Server.ContextPath
	Server.BaseUrl = config.Server.BaseUrl

	return nil
}
