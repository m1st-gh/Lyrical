FROM golang:1.23

# Set the working directory to the root directory
WORKDIR /

# Pre-copy/cache go.mod for pre-downloading dependencies and only redownloading them in subsequent builds if they change
COPY go.mod go.sum ./
RUN go mod download && go mod verify

# Copy the entire application to the root directory
COPY . .

# Build the Go application and output the binary directly to the root (/)
RUN go build

CMD ["./Lyrical"]
