# Assignment 6: DNS Query

This is a simple DNS query program that uses raw sockets to send DNS queries to a DNS server and receive the response. The program is divided into two parts: the server and the client. The server listens for incoming DNS queries and responds with the IP address of the requested domain. The client sends DNS queries to the server and displays the IP address of the requested domain.

## Building and Running the Server

1. To compile the server, use the provided Makefile:

    ```bash
    make server
    ```

2. To run the server, use the following command. We use `sudo` because raw sockets require root privileges:

    ```bash
    sudo ./server
    ```

## Building and Running the Client

1. To compile the client, use the provided Makefile:

    ```bash
    make client
    ```

2. To run the client, use the following command. We use `sudo` because raw sockets require root privileges:

    ```bash
    sudo ./client
    ```

3. After running the client, you can ask for the IP of a domain in the following format:

    ```bash
    getIP N <domain-1> <domain-2> <domain-3> … <domain-N>
    ```

    Replace `N` with the number of domains you want to query, and `<domain-1>`, `<domain-2>`, etc., with the actual domain names.

4. To exit the client, type in:

    ```bash
    EXIT
    ```