package main

import (
	"bufio"
	"flag"
	"fmt"
	"math/rand"
	"net"
	"os"
	"path"
	"sort"
	"strconv"
	"strings"
	"time"
)

var (
	host string
	port string
)

func main() {
	flag.StringVar(&host, "host", "localhost", "IP adress of the server")
	flag.StringVar(&port, "port", "21", "Port of the server")
	flag.Parse()
	rand.Seed(time.Now().UTC().UnixNano())
	reader := bufio.NewReader(os.Stdin)
	if len(os.Args) == 1 {
		fmt.Println("Server address not specified")
		os.Exit(1)
	}
	var err error
	handler := Handler{}
	fmt.Println("Connecting to " + host + "......")
	connection, err := net.Dial("tcp", host+":"+port)
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
	fmt.Println(string(username))
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
			if handler.passive {
				conn, resp := handler.CmdPassive("retr " + inputList[1])
				if strings.HasPrefix(resp, "150") {
					_, fileName := path.Split(inputList[1])
					file, _ := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY, 0755)
					handler.DataResponse(conn, file)
					file.Close()
					conn.Close()
					fmt.Println(handler.InstResponse())
				}
			} else {
				func() {
					lis, resp := handler.CmdPositive("retr " + inputList[1])
					defer lis.Close()
					if strings.HasPrefix(resp, "150") {
						conn, _ := lis.Accept()
						_, fileName := path.Split(inputList[1])
						file, _ := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY, 0755)
						handler.DataResponse(conn, file)
						file.Close()
						conn.Close()
						fmt.Println(handler.InstResponse())
					}
				}()
			}
		} else if cmdstr == "put" {
			if len(inputList) < 2 {
				goto label
			}
			if _, err := os.Stat(inputList[1]); os.IsNotExist(err) {
				goto label
			}
			_, fileName := path.Split(inputList[1])
			if handler.passive {
				conn, resp := handler.CmdPassive("stor " + fileName)
				if strings.HasPrefix(resp, "150") {
					file, _ := os.OpenFile(inputList[1], os.O_RDONLY, 0755)
					handler.DataRequest(conn, file)
					file.Close()
					conn.Close()
					fmt.Println(handler.InstResponse())
				}
			} else {
				func() {
					lis, resp := handler.CmdPositive("stor " + fileName)
					defer lis.Close()
					if strings.HasPrefix(resp, "150") {
						conn, _ := lis.Accept()
						file, _ := os.OpenFile(inputList[1], os.O_RDONLY, 0755)
						handler.DataRequest(conn, file)
						file.Close()
						conn.Close()
						fmt.Println(handler.InstResponse())
					}
				}()
			}
		} else if cmdstr == "ls" {
			cmdfull := "list"
			if len(inputList) == 2 {
				cmdfull = cmdfull + " " + inputList[1]
			}
			if handler.passive {
				conn, resp := handler.CmdPassive(cmdfull)
				if strings.HasPrefix(resp, "150") {
					handler.DataResponse(conn, nil)
					conn.Close()
					fmt.Println(handler.InstResponse())
				}
			} else {
				func() {
					lis, resp := handler.CmdPositive(cmdfull)
					defer lis.Close()
					if strings.HasPrefix(resp, "150") {
						conn, _ := lis.Accept()
						handler.DataResponse(conn, nil)
						conn.Close()
						fmt.Println(handler.InstResponse())
					}
				}()
			}

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
		} else if cmdstr == "restart" {
			if len(inputList) < 2 {
				goto label
			}
			_, fileName := path.Split(inputList[1])
			file, _ := os.OpenFile(fileName, os.O_RDWR|os.O_CREATE, 0755)
			info, _ := file.Stat()
			fmt.Println("rest " + strconv.Itoa(int(info.Size())))
			handler.InstRequest("rest " + strconv.Itoa(int(info.Size())))
			fmt.Println(handler.InstResponse())
			file.Close()
			if handler.passive {
				conn, resp := handler.CmdPassive("retr " + inputList[1])
				if strings.HasPrefix(resp, "150") {
					_, fileName := path.Split(inputList[1])
					file, _ := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0755)
					handler.DataResponse(conn, file)
					file.Close()
					conn.Close()
					fmt.Println(handler.InstResponse())
				}
			} else {
				func() {
					lis, resp := handler.CmdPositive("retr " + inputList[1])
					defer lis.Close()
					if strings.HasPrefix(resp, "150") {
						conn, _ := lis.Accept()
						_, fileName := path.Split(inputList[1])
						file, _ := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0755)
						handler.DataResponse(conn, file)
						file.Close()
						conn.Close()
						fmt.Println(handler.InstResponse())
					}
				}()
			}
		} else {
			goto label
		}
		continue
	label:
		fmt.Println("Command failed")
	}
}
