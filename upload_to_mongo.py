#!/usr/bin/env python3

import logging
import sys
import os
from pymongo import MongoClient
import pymongo.errors
import argparse
import json

logger = logging.getLogger(__name__)

def preprocess(d):
    return {k.replace('.', '_') : preprocess(v) for k,v in d.items()} if isinstance(d,dict) else d


def upload_to_mongodb(address, port, username, password, db_name, collection, data):
    
    logger = logging.getLogger("mongodb")
    client = MongoClient(address, int(port), username=username, password=password)
    with client:
        db = client[db_name]
        collection = db[collection]
        result = collection.insert_one(preprocess(data))
        logger.info(f"Inserted: {result} into {address}:{port}/{db_name}")

def upload_to_gridfs(address, port, username, password, db_name, collection, data):
    
    logger = logging.getLogger("mongodb")
    client = MongoClient(address, int(port), username=username, password=password)
    with client:
        db = client[db_name]
        fs = gridfs.GridFS(db, collection=collection)
        return fs.put(json.dumps(data, indent=4), encoding='utf-8')


if __name__ == "__main__":
    help_msg = "custom uploader to mongodb"
    # Parse arguments
    parser = argparse.ArgumentParser(
        description=help_msg, formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("json_path")
    args = parser.parse_args()

    logger = logging.getLogger("uploader")
    # Setup database
    db_address = db_port = db_user = db_passwd = db_name = db_collection = None
    try:
        db_address = os.environ["MONGO_ADDRESS"]
        db_port = os.environ["MONGO_PORT"]
        db_user = os.environ["MONGO_USER"]
        db_passwd = os.environ["MONGO_PASSWORD"]
        db_name = os.environ["MONGO_DB_NAME"]
        db_collection = os.environ["MONGO_DB_COLLECTION"]
    except KeyError as e:
        logger.warning(
            f"Environment variable {e} was not specified, so results cannot be uploaded to the database"
        )
    with open(args.json_path) as json_file:
        data = json.load(json_file)
        upload_to_mongodb(
               db_address, db_port, db_user, db_passwd, db_name, db_collection, data) 

    
