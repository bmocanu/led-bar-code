package model

type ServerConfig struct {
	ListenAddress string
	ListenPort    int
	ContextPath   string
	BaseUrl       string
}
