docker run -it --name jungle-container -v %cd%:/home/jungle/workspace jungle-image bash

docker build -t jungle-image .