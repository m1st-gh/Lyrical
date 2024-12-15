package main

import "log"

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
