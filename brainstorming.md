# Brainstorming

This file is used to document your thoughts, approaches and research conducted across all tasks in the Technical Assessment.

## Firmware

## Telemetry

## Cloud

### **Requirements and Assumptions**
- Current systems run AWS products. So it is preferable to use other AWS products. 
- The weather system that is being built must monitor temperature, air humidity, track temperature, wind, and wind direction
- Justify the design decisions made, including budget, security, performance and scalability

### **Purpose**
System to track and correlate weather data with vehicle performance

### **Brainstormed Points/Research Points/Extrapolations**
- Entire server uses dynamoDB (key-value NoSQL). Combining them together in an SQL database is useful for telemetry analysis.
- As the purpose of weather station is for data analytics, it should only collect data when the car is running
- Provide a way of access with UI for engineers. Good inspiration should be a drag and drop form of data analytics like tableau
- System should provide cost over scalability

### **Assumptions**
- Load Balancer is able to handle 2k requests per second (1000 per second for firmware and weather station)
- Gateway service is able to handle 2k request per second 
- All data is gathered on a milisecond basis
- Weather station should transmit data in CAN form

### **Final Product**
- Added weather station (1 snapshot of data per ms).
 The weather station outputs CAN data where the different data can be temperature, air, himidity, track temperature, wind, and wind direction.
- Uses same load balancer for weather station.
- Gateway service doubles for both weather-streaming-service and firmware-streaming-service.
- Added redis and ECS container for in memory storage of weather data. ECS containers can be initialized with docker images and swapped out with other docker images. Appropraite configuration of service is later setup with terraform.
- S3 bucket for log files and new dynamoDB for weather data with read-and-write-public access.

### *Analytics part*
- Lambda accesses weather and firmware dynamoDB databases
- Athena can join together two databases matched together with unix timestamp (refer to source 1 for more documentation)
Ex:
Where
```
SELECT * FROM weather
SELECT * FROM telemetry
```
Then you can join them together such that
```
SELECT time, temperature, humidity, ... from weather 
    JOIN telemetry ON weather.time = telemetry.time;
```
- Joined database added into the s3 bucket. Each database represents a "race session" or "run time"
- API calls can get AWS QuickSight which returns graphs to the front end for engineers to use
- Optionally, users can also query Athena directly. 

### **Terraform Deployment**
Terraform can be used to deploy and configure the new technologies (and the old which exists) in the system. In this case configuring:
1. S3 bucket for log files.
Terraform can be used to configure the S3 buckets with versioning and logging. Then add Add IAM policies for guarding the bucket against unallowed access.
Example Configuration:
```
resource "aws_s3_bucket" "weather_bucket" {
  bucket = "weather-data-bucket"
  acl = "public-read-write"
}

resource "aws_s3_bucket_policy" "bucket_policy" {
  bucket = aws_s3_bucket.weather_bucket.id
  policy = data.aws_iam_policy_document.read_and_write_public.json
}

data "aws_iam_policy_document" "read_and_write_public" {
  statement {
    principals {
      type        = "AWS"
      identifiers = ["683633011377"]
    }
    
    actions = [
      "s3:GetObject",
      "s3:ListBucket",
      "s3:PutObject"
    ]
  
    resources = [
      aws_s3_bucket.weather_bucket.arn,
      "${aws_s3_bucket.weather_bucket.arn}/*",
    ]
  }
}
```
<br>

2. New ECS containers redis and weather-streaming
Terraform can be used here to add new task-definitions to implment a new docker image (suited for the new weather machine). In this case the tasks are weather-streaming-service and redis. These tasks are then implemented by a service configuration which makes them happen. 

Example of weather-streaming-service: (Credit to reference 10 for large terraform code reference)
```
resource "aws_ecs_task_definition" "weather-streaming-task" {
  family                   = "weather-streaming" 
  container_definitions    = <<DEFINITION
  [
    {
      "name": "app-first-task",
      "image": "${aws_ecr_repository.app_ecr_repo.repository_url}",
      "essential": true,
      "portMappings": [
        {
          "containerPort": 5000,
          "hostPort": 5000
        }
      ],
      "memory": 512,
      "cpu": 256
    }
  ]
  DEFINITION
  requires_compatibilities = ["FARGATE"] 
  network_mode             = "awsvpc"    
  memory                   = 512         
  cpu                      = 256         
  execution_role_arn       = "${aws_iam_role.ecsTaskExecutionRole.arn}"
}

resource "aws_ecs_service" "weather_streaming_service" {
  name            = "weather_streaming_service"     
  cluster         = "${aws_ecs_cluster.my_cluster.id}" 
  task_definition = "${aws_ecs_task_definition.weather-streaming.arn}" 
  launch_type     = "FARGATE"
  desired_count   = 1

  network_configuration {
    subnets          = ["${aws_default_subnet.default_subnet_a.id}", "${aws_default_subnet.default_subnet_b.id}"]
    assign_public_ip = true     # Provide the containers with public IPs
    security_groups  = ["${aws_security_group.service_security_group.id}"] 
  }
}

```
<br>

3. DynamoDB for weather data
The dynamoDB should use terraform to configure the dynamodb read and write capacity, attributes, time to live (ttl), and life cycle. Then once this is done then add a CRUD api following this such that the dynamodb can be updated as required. 
```
provider "aws" {
  region = "ap-southeast-2"
  profile = "1.1"
}

resource "aws_dynamodb_table" "basic-dynamodb-table" {
  name           = "weather-dynamodb-table"
  billing_mode   = "PROVISIONED"
  read_capacity  = 1000
  write_capacity = 1000
  hash_key       = "TimeStamp"

  attribute {
    name = "TimeStamp"
    type = "N"
  }

  ttl {
    attribute_name = "TimeToExist"
    enabled        = true
  }

  global_secondary_index {
    name               = "TimeStampIndex"
    hash_key           = "TimeStamp"
    write_capacity     = 1000
    read_capacity      = 1000
    projection_type    = "INCLUDE" 
    non_key_attributes = ["temperature", "air_humidity", "track_temperature", "wind", "wind_direction"]
  }

  tags = {
    Name        = "weather-dynamodb-table"
    Environment = "production"
  }
}
```

It can also modify the following:

4. Existing load balancer
5. Gateway streaming service

The gateway streaming service can have it's streaming be modified to include weather data while the existing load balancer can include the weather-streaming-service as another avenue and the weather station as a source node. 

### **Security Analysis**
Given that Athena, new S3 buckets, new dynamoDB databases, and AWS quicksight are properly configured with correct AMI then they should only be accessed with people who have those cridentials. Services new containers and services setup in the ECS Fargate Clusters should be configured with the appropriate security group settings. This should allow the different amazon AWS services to be connected appropriately. 

### **Cost Analysis**
All pricing is based on Sydney pricing:

Fargate Cost for two tasks where (x86/Linux)
- weather-streaming-service: 0.5vCPU with 1GB ram -> $0.0000082223/s
- redis: 1vCPU with 2GB ram -> $0.0000164445/s

Lambda Costs: 
- $0.20 per 1M requests

DynamoDB (10000 WCU and RCU):
- Write Cost: $0.0018055556/s
- Read Cost: $0.0003611111/s

S3 Bucket Pricing: 
- First 50 TB / Month $0.025 per GB

Amazon QuickSight:
- Author: $24 per user
- Reader: $3 per reader

Amazon Athena:
- SQL with Provisional Queries: $0.36 DPU per hour 

*Total Cost Analysis* (tables should be read in markdown reader)

Monthly Upkeep
|Name                       |Singlular Cost |Expected Use   |Total Cost|
|---|---|---|---|
|Amazon Quicksight Author   |$24            |1              |$24
|Amazon Quicksight Reader   |$3             |1              |$3
|S3 Buckets (below 50TB)    |$0.025 per GB  |1100           |$27.5
|Total                      ||                              |$54.5

Per Run/Race Upkeep (5 Minute per run assumption)
|Name                       |Singlular Cost         |Expected Use   |Total Cost|
|---|---|---|---|
|weather-streaming-service  |$0.0000082223/s        |300            |$0.00246669
|redis                      |$0.0000164445/s        |300            |$0.00493335
|Lambda (weather log)       |$0.20 per 1M requests  | 300,000       |$0.06
|Lambda (athena request)    |$0.20 per 1M requests  | 500           |$0
|DynamoDB Writes            |$0.0018055556/s        |300            |$0.54166668 
|DynamoDB Reads             |$0.0003611111/s        |300            |$0.10833333
|Total                      ||                                      |$0.71740005 / per car test, race, per 5 minutes               

Cost Notes:
- Assumes all data is taken in 1ms intervals for 1000 points of can data a second for each source node. 
- Large cost savings can be done through not using Amazon QuickSight and perhaps making a self implementation of a telemetry system

### Important References:

Athena Resources:
1. https://aws.amazon.com/blogs/database/how-to-perform-advanced-analytics-and-build-visualizations-of-your-amazon-dynamodb-data-by-using-amazon-athena/
2. https://docs.aws.amazon.com/prescriptive-guidance/latest/patterns/access-query-and-join-amazon-dynamodb-tables-using-athena.html

IaC Resources:
3. https://kodekloud.com/blog/how-to-create-aws-s3-bucket-using-terraform/
4. https://dynobase.dev/dynamodb-terraform/
5. https://spacelift.io/blog/terraform-ecs
6. https://serverfault.com/questions/843498/terraform-how-do-i-have-1-ecs-cluster-with-2-or-more-ecs-service-task-definition
7. https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/dynamodb_table
8. https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role
9. https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ecs_task_definition
10. https://earthly.dev/blog/deploy-dockcontainers-to-awsecs-using-terraform/ (really good)

Pricing Resource:
11. https://aws.amazon.com/fargate/pricing/
12. https://aws.amazon.com/quicksight/pricing/
13. https://aws.amazon.com/athena/pricing/
14. https://aws.amazon.com/dynamodb/pricing/provisioned/
15. https://aws.amazon.com/s3/pricing/
16. https://aws.amazon.com/lambda/pricing/
