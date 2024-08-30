# Brainstorming

This file is used to document your thoughts, approaches and research conducted across all tasks in the Technical Assessment.

This file is written in point form of
- Brainstorming Notes
- Assumptions (made when making the product)
- and an explanation in section "final product"
- Retrospect reviewing what I think I could have done differently

(it also helps alot to see this brainstorm.md in markdown reader)

## Firmware 
Please compile with make file by typing
`make clean && make` in the firmware directory

### Brainstorming Notes

#### Notes
Form in dump.log:
Timestamp id#data
Unix timestamp, 11 bit id (705 max for sensorbus.dbc) and 64 bit data.

DBC standard notes:
id can be upto 32 bits data is always can be upto 1785 bits
can-interface can be different (slcan0, vcan0, etc) as seen in Nissan Leaf Can file ([Link](https://raw.githubusercontent.com/aphrx/canx/master/dumps/nissan_leaf_candump.log))

Ex:
(1705638751.941934) vcan0 474#8EB6967DC359
#### Requirements
Requirements: 
- Result: Must be fast. Reading DBC can be done at moderate speed but only read once. These values can then be loaded into memory in some form and fed into a log debugger. 
- System: Sort of minimum-viable-product (MVP) style where adding functions, objects, DBC scanner can be added
#### Design (include actual pictures from my notebook)
![](images/Screenshot%202024-08-29%20at%2022.27.11.JPG)
1. DBC file should be processed by a DBC processor which takes it apart
2. The DBC file should generate a template on the information required to build the file
3. The CAN processor should take the instructions and log file together
4. The CAN processor then outputs directly to the file

### Assumptions
- DBC files can be hard coded as values
- Data is always 64 bit
- id can be of any length
- log is always of form (TIME) ID#DATA

### Final Product
Three main files:
- main.cpp -> start to start and run everything
- dbcUtil.h -> and attributes of can_instruction and an extract dbc function which maps dbc_ids to can_instructions
- LogUtil.h -> functions that take attributes can_instructions a log_file and outputs them in a specified format into output.txt

Currently the program decodes and implements
1. Big/Little endian CAN data
2. 8,16,32,64 bit size CAN signal
3. Scale and offset

The program does not decode and implement: 
1. min/max
2. signed data

However, the program provides more space to extend into reading abitrary dbc files and adding min/max and signed data decoding.

### Retrospect
- Change the dbcUtil to create an object with a decorator pattern such that it builds objects "instruction objects" for the LogUtil.
- instead of .cpp and .h I should have used a .hpp to add all of them together as c++ source code header file to make life easier. 

## Telemetry

### Brainstorming Notes
#### Error Notes
Errors:
streaming-service-1  | Received: {"battery_temperature":54.437145812093874,"timestamp":1724950337441}}
streaming-service-1  | Received: {"battery_temperature":27.05925502865341,"timestamp":1724950278073}}
streaming-service-1  | Received: {"battery_temperature":66.58650380035478,"timestamp":1724949902715}}

They all occur at:
streaming-service-1  | /app/src/server.ts:20
streaming-service-1  |     const jsonData: VehicleData = JSON.parse(msg.toString());
streaming-service-1  |                                        ^
streaming-service-1  | SyntaxError: Unexpected token } in JSON at position 67
streaming-service-1  |     at JSON.parse (<anonymous>)
streaming-service-1  |     at Socket.<anonymous> (/app/src/server.ts:20:40)
streaming-service-1  |     at Socket.emit (node:events:517:28)
streaming-service-1  |     at Socket.emit (node:domain:489:12)
streaming-service-1  |     at addChunk (node:internal/streams/readable:368:12)
streaming-service-1  |     at readableAddChunk (node:internal/streams/readable:341:9)
streaming-service-1  |     at Socket.Readable.push (node:internal/streams/readable:278:10)
streaming-service-1  |     at TCP.onStreamRead (node:internal/stream_base_commons:190:23)

So there the error comes from an additional } at the end consistently.

#### Safe Operating Temperature Tracking
- Use a deque system to add into a stack while having first in first out (FIFO) capability
- dequeue system must be fast because it is called often
- dequeue system keeps track of temperatures too high for each time they are added and taken out

### Assumptions
- Errors are only from dounle curly braces (}}) instead of general 

### Final Product
1. Error Fix:
The error is identified as an extra backet at the end of the msg and thus produces a problem
for the JSON file. The fix is
```ts
var jsonData: VehicleData;
var client_message: string;
try {
  jsonData = JSON.parse(msg.toString());
  client_message = msg.toString();
} catch (SyntaxError) {
  console.log("Error handled, extra } removed");
  jsonData = JSON.parse(msg.toString().substring(0, msg.length - 1))
  client_message = msg.toString().substring(0, msg.length - 1);
} 
```
2. A tracking system is added which tracks all times for the last five seconds. For every new item which is added. It displays the message 
`streaming-service-1  | (%d) Unsafe Battery Limit Reached`
upon seeing that there are too much unsafe battery signals

A debug function is also kept in the code which is commented out. In production this should not be added. However, I left it in for demonstration. 

Uncomment server.ts line 41-42 and deque.ts 76-85. The following code is produced whenever there are too many reports of a hot battery. 
```
streaming-service-1  | Received: {"battery_temperature":970,"timestamp":1725025445575}
streaming-service-1  | (%d) Unsafe Battery Limit Reached
streaming-service-1  | Current Log:
streaming-service-1  | 1725025441045 18 <--- !!!!
streaming-service-1  | 1725025441553 76.78533398236367
streaming-service-1  | 1725025442056 566 <--- !!!!
streaming-service-1  | 1725025442557 28.55725850109278
streaming-service-1  | 1725025443061 68.13089256487677
streaming-service-1  | 1725025443562 28.97543745332059
streaming-service-1  | 1725025444066 744 <--- !!!!
streaming-service-1  | 1725025444568 64.16224207762622
streaming-service-1  | 1725025445072 79.2952312921794
streaming-service-1  | 1725025445575 970 <--- !!!!
streaming-service-1  | Count Errors: 4
```
3. Adding the color change 
Change the code for live_value.tsx such that
```tsx
function LiveValue( temp : TemperatureProps) {
  if(temp.temp >= 20 && temp.temp <= 80) {
    return (
      <header className="live-value" style={{ color: "white" }}>
        {`${temp.temp.toPrecision(3)}°C`}
      </header>
    );
  } else {
    return (
      <header className="live-value" style={{ color: "red" }}>
        {`${temp.temp.toPrecision(3)}°C`}
      </header>
    );
  }
}
```
### Retrospect
- Spend more time on typescript type safety
- Do more UI changes for code

## Cloud

### Brainstorming Notes
#### Requirements
- Current systems run AWS products. So it is preferable to use other AWS products. 
- The weather system that is being built must monitor temperature, air humidity, track temperature, wind, and wind direction
- Justify the design decisions made, including budget, security, performance and scalability
#### Purpose
System to track and correlate weather data with vehicle performance
#### Brainstormed Points/Research Points/Extrapolations
- Entire server uses dynamoDB (key-value NoSQL). Combining them together in an SQL database is useful for telemetry analysis.
- As the purpose of weather station is for data analytics, it should only collect data when the car is running
- Provide a way of access with UI for engineers. Good inspiration should be a drag and drop form of data analytics like tableau
- System should provide cost over scalability

### Assumptions
- Load Balancer is able to handle 2k requests per second (1000 per second for firmware and weather station)
- Gateway service is able to handle 2k request per second 
- All data is gathered on a milisecond basis
- Weather station should transmit data in CAN form

### Final Product (inside "Cloud Diagram Modified.pdf")
- Added weather station (1 snapshot of data per ms).
 The weather station outputs CAN data where the different data can be temperature, air, himidity, track temperature, wind, and wind direction.
- Uses same load balancer for weather station.
- Gateway service doubles for both weather-streaming-service and firmware-streaming-service.
- Added redis and ECS container for in memory storage of weather data. ECS containers can be initialized with docker images and swapped out with other docker images. Appropraite configuration of service is later setup with terraform.
- S3 bucket for log files and new dynamoDB for weather data with read-and-write-public access.

*Analytics part*
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

### Terraform Deployment
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

### Security Analysis
Given that Athena, new S3 buckets, new dynamoDB databases, and AWS quicksight are properly configured with correct AMI then they should only be accessed with people who have those cridentials. Services new containers and services setup in the ECS Fargate Clusters should be configured with the appropriate security group settings. This should allow the different amazon AWS services to be connected appropriately. 

### Cost Analysis
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
- SQL with Provisional Queries: |$5 / TB Scanned

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
|Amazon Athena              |$5 / TB Scanned        |0.5GB per 5 min|$0.00025
|Total                      ||                                      |$0.71765005 / per 5 minutes               

Cost Notes:
- Assumes all data is taken in 1ms intervals for 1000 points of can data a second for each source node. 
- Large cost savings can be done through not using Amazon QuickSight and perhaps making a self implementation of a telemetry system

### Retrospect
What I would have done differently:
- Replace Amazonquick sight with a custom system to visualize data to save money
- Look deeper into the need (or lackof) of Amazon Fire Hose

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
