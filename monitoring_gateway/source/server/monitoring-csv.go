package serve

import (
	log "github.com/sirupsen/logrus"
	"io"
	"math/rand"
	"net/http"
	"strconv"
)

func getMonitoringCsv(response http.ResponseWriter, request *http.Request) {
	log.Info("HTTP Get for the current monitoring CSV")
	var pageContent = "status,100\n"

	for row := range 3 {
		var pixelsPerRow = 10 // rand.Intn(11) + 1
		for range pixelsPerRow {
			var red = rand.Intn(255)
			var green = rand.Intn(255)
			var blue = rand.Intn(255)
			var rgb = red
			rgb = (rgb << 8) + green
			rgb = (rgb << 8) + blue
			var pixel = rand.Intn(53)
			pageContent +=
				strconv.Itoa(row) + "," +
					strconv.Itoa(pixel) + "," +
					strconv.Itoa(rgb) + "\n"
		}
	}

	setContentTypeHeader(response, "text/csv")
	_, err := io.WriteString(response, pageContent)
	if err != nil {
		log.Error("Failed to write monitoring CSV content to HTTP response: ", err)
		response.WriteHeader(http.StatusInternalServerError)
	}
}
