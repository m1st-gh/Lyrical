package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"runtime"
	"strings"
	"time"
)

func SetLoggerOutput(fileName string) {
	startTime := time.Now().Format("01_02_2006_15-04-05")
	fileNameWithTime := fmt.Sprintf("%s_%s.log", fileName, startTime)
	logFile, err := os.OpenFile(fileNameWithTime, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		log.Fatalf("Failed to open log file: %v", err)
	}
	log.SetFlags(log.LstdFlags)
	multiWriter := io.MultiWriter(os.Stdout, logFile)
	log.SetOutput(multiWriter)
}

func Warn(format string, v ...interface{}) {
	logMessage("[WARN]", format, v...)
}

func Info(format string, v ...interface{}) {
	logMessage("[INFO]", format, v...)
}

func Error(format string, v ...interface{}) {
	logMessage("[ERROR]", format, v...)
}

func Fatal(format string, v ...interface{}) {
	logAndExit("[FATAL]", format, v...)
}

func Panic(format string, v ...interface{}) {
	logAndPanic("[PANIC]", format, v...)
}

func logMessage(level, format string, v ...interface{}) {
	_, file, line, ok := runtime.Caller(2)
	if !ok {
		file = "unknown"
		line = 0
	}
	fileParts := strings.Split(file, "/")
	fileName := fileParts[len(fileParts)-1]
	log.Printf("%s %s:%d\t%s\n", level, fileName, line, fmt.Sprintf(format, v...))
}

func logAndExit(level, format string, v ...interface{}) {
	_, file, line, ok := runtime.Caller(2)
	if !ok {
		file = "unknown"
		line = 0
	}
	fileParts := strings.Split(file, "/")
	fileName := fileParts[len(fileParts)-1]
	log.Printf("%s %s:%d\t%s\n", level, fileName, line, fmt.Sprintf(format, v...))
	os.Exit(1)
}

func logAndPanic(level, format string, v ...interface{}) {
	_, file, line, ok := runtime.Caller(2)
	if !ok {
		file = "unknown"
		line = 0
	}
	fileParts := strings.Split(file, "/")
	fileName := fileParts[len(fileParts)-1]
	panic(fmt.Sprintf("%s %s:%d\t%s", level, fileName, line, fmt.Sprintf(format, v...)))
}
