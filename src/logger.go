package main

import (
	"io"
	"log"
	"os"
)

func SetLoggerOutput(fileName string) {
	logFile, err := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		Panic("%v", err)
	}
	mw := io.MultiWriter(os.Stdout, logFile)
	log.SetOutput(mw)
}

func Warn(format string, v ...interface{}) {
	log.Printf("[WARN] "+format+"\n", v...)
}

func Info(format string, v ...interface{}) {
	log.Printf("[INFO] "+format+"\n", v...)
}

func Error(format string, v ...interface{}) {
	log.Printf("[ERROR] "+format+"\n", v...)
}

func Fatal(format string, v ...interface{}) {
	log.Fatalf("[FATAL] "+format+"\n", v...)
}
func Panic(format string, v ...interface{}) {
	log.Panicf("[PANIC] "+format+"\n", v...)
}
