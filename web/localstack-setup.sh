#!/bin/bash

aws --endpoint-url=http://localhost:4566 s3 mb s3://nutc
aws --endpoint-url=http://localhost:4566 s3api put-bucket-cors --bucket nutc --cors-configuration file://cors.json

#aws --endpoiunt-url=http://localhost:4566 s3 ls s3://nutc --recursive
