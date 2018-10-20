package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
	"path"
	"sort"
	"strings"
)

func main() {
	reader := bufio.NewReader(os.Stdin)
	if len(os.Args) == 1 {
		fmt.Println("Server address not specified")
		os.Exit(1)
	}
	var err error
	handler := Handler{}
	fmt.Println("Connecting to " + os.Args[1] + "......")
	connection, err := net.Dial("tcp", os.Args[1]+":21")
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	handler.connection = connection
	fmt.Println(connection.LocalAddr(), connection.RemoteAddr())
	handler.passive = true
	response := handler.InstResponse()
	fmt.Println(response)
	fmt.Print("Username:")
	username, _, _ := reader.ReadLine()
	handler.InstRequest("user " + string(username))
	response = handler.InstResponse()
	fmt.Println(response)
	if strings.HasPrefix(response, "331") {
		fmt.Print("Password:")
		password, _, _ := reader.ReadLine()
		handler.InstRequest("pass " + string(password))
		fmt.Println(handler.InstResponse())
	}
	for {
		beginstr := "ftp> "
		if handler.passive {
			beginstr = "[mode  passive]" + beginstr
		} else {
			beginstr = "[mode positive]" + beginstr
		}
		fmt.Print(beginstr)
		input, _, _ := reader.ReadLine()
		inputList := strings.Fields(string(input))
		if len(inputList) == 0 {
			continue
		}
		cmdstr := strings.ToLower(inputList[0])
		command, ok := COMMANDLIST[cmdstr]
		if ok && command.argc == 0 {
			handler.InstRequest(command.text)
			response := handler.InstResponse()
			fmt.Println(response)
			if strings.HasPrefix(response, "221") {
				os.Exit(0)
			}
		} else if ok && command.argc == 1 {
			if len(inputList) < 2 {
				goto label
			}
			handler.InstRequest(command.text + " " + inputList[1])
			fmt.Println(handler.InstResponse())
		} else if cmdstr == "passive" {
			handler.passive = true
			fmt.Println("Passive mode successfully configured")
		} else if cmdstr == "positive" {
			handler.passive = false
			fmt.Println("Positive mode successfully configured")
		} else if cmdstr == "help" {
			fmt.Println("Supported commmands:")
			list := StringSlice{"put", "get", "mv", "help", "ls"}
			for key := range COMMANDLIST {
				list = append(list, key)
			}
			sort.Sort(list)
			for _, v := range list {
				fmt.Println(v)
			}
		} else if cmdstr == "get" {
			if len(inputList) < 2 {
				goto label
			}
			var conn net.Conn
			var resp string
			if handler.passive {
				conn, resp = handler.CmdPassive("retr " + inputList[1])
			} else {
				conn, resp = handler.CmdPositive("retr " + inputList[1])
			}
			if !strings.HasPrefix(resp, "150") {
				return
			}
			_, fileName := path.Split(inputList[1])
			handler.DataResponse(conn, fileName)
			conn.Close()
			fmt.Println(handler.InstResponse())
		} else if cmdstr == "put" {
			if len(inputList) < 2 {
				goto label
			}
			if _, err := os.Stat(inputList[1]); os.IsNotExist(err) {
				goto label
			}
			_, fileName := path.Split(inputList[1])
			var conn net.Conn
			var resp string
			if handler.passive {
				conn, resp = handler.CmdPassive("stor " + fileName)
			} else {
				conn, resp = handler.CmdPositive("stor " + fileName)
			}
			if !strings.HasPrefix(resp, "150") {
				return
			}
			handler.DataRequest(conn, inputList[1])
			conn.Close()
			fmt.Println(handler.InstResponse())
		} else if cmdstr == "ls" {
			cmdfull := "list"
			if len(inputList) == 2 {
				cmdfull = cmdfull + " " + inputList[1]
			}
			func() {
				var conn net.Conn
				var resp string
				if handler.passive {
					conn, resp = handler.CmdPassive(cmdfull)
				} else {
					conn, resp = handler.CmdPositive(cmdfull)
				}
				defer conn.Close()
				if !strings.HasPrefix(resp, "150") {
					return
				}
				handler.DataResponse(conn, "")
				fmt.Println(handler.InstResponse())
			}()
		} else if cmdstr == "mv" {
			if len(inputList) < 3 {
				goto label
			}
			handler.InstRequest("rnfr " + inputList[1])
			response := handler.InstResponse()
			fmt.Println(response)
			if !strings.HasPrefix(response, "350") {
				continue
			}
			handler.InstRequest("rnto " + inputList[2])
			fmt.Println(handler.InstResponse())
		} else {
			goto label
		}
		continue
	label:
		fmt.Println("Command failed")
	}
}
