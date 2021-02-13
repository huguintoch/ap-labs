package main

import (
    "fmt"
    "os"
)

func main() {

    if (len(os.Args) < 2) {
      fmt.Println("[ERROR]: Please type name(s) as argument")
    } else {
      var msg = "Hello "
      for _, i := range os.Args[1:] {
        msg += i + " "
      }
      msg = msg[:len(msg)-1]
      msg += ", Welcome to the jungle"
      fmt.Println(msg)
    }

}