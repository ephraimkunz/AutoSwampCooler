from __future__ import print_function # Python 2/3 compatibility
import boto3

dynamodb = boto3.resource('dynamodb', region_name='us-east-1', endpoint_url="https://dynamodb.us-east-1.amazonaws.com")


table = dynamodb.create_table(
    TableName='SwampData',
    KeySchema=[
        {
            'AttributeName': 'timestamp',
            'KeyType': 'HASH'  #Partition key
        },
        {
            'AttributeName': 'data',
            'KeyType': 'RANGE'  #Sort key
        }
    ],
    AttributeDefinitions=[
        {
            'AttributeName': 'timestamp',
            'AttributeType': 'N'
        },
        {
            'AttributeName': 'data',
            'AttributeType': 'S'
        },

    ],
    ProvisionedThroughput={
        'ReadCapacityUnits': 10,
        'WriteCapacityUnits': 10
    }
)

print("Table status:", table.table_status)
