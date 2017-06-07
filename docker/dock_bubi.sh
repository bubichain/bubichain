echo "copy bubi..." &&
cp -f ../src/main/bubi blockchain/bin/bubi &&
echo "removing container..." &&
docker rm bbc &&
echo "building container..." &&
docker build -t bbc:latest . &&
echo "run -d --name bbc bbc:latest" &&
docker run -d --name bbc bbc:latest
