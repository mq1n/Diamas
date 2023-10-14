#!/bin/bash

# Check if the script has already been executed
if [ -f /tmp/migration_complete ]; then
    echo "Migration script has already been executed. Exiting."
    exit 0
fi

# # Retrieve MySQL server connection parameters from environment variables
MYSQL_HOST="$MYSQL_HOST_ENV"
MYSQL_ROOT_PASSWORD="$MYSQL_ROOT_PASSWORD_ENV"
MYSQL_USER="$MYSQL_USER_ENV"
MYSQL_PASSWORD="$MYSQL_PASSWORD_ENV"

# echo "MYSQL_HOST: $MYSQL_HOST"
# echo "MYSQL_ROOT_PASSWORD: $MYSQL_ROOT_PASSWORD"
# echo "MYSQL_USER: $MYSQL_USER"
# echo "MYSQL_PASSWORD: $MYSQL_PASSWORD"

# Check if required environment variables are set
if [ -z "$MYSQL_HOST" ] || [ -z "$MYSQL_ROOT_PASSWORD" ]  || [ -z "$MYSQL_USER" ] || [ -z "$MYSQL_PASSWORD" ]; then
    echo "One or more required environment variables are not set."
    exit 1
fi

# Wait for MySQL to be ready
until mariadb -h"$MYSQL_HOST" -u"root" -p"$MYSQL_ROOT_PASSWORD" -e "SELECT 1"; do
  echo "Waiting for MySQL to become available..."
  sleep 5
done

# SQL queries
QUERIES=(
    # "GRANT ALL PRIVILEGES ON *.* TO root@"%" IDENTIFIED BY "123";"
    # "GRANT GRANT OPTION ON *.* TO root@"%";"
    "CREATE DATABASE IF NOT EXISTS account;"
    "CREATE DATABASE IF NOT EXISTS common;"
    "CREATE DATABASE IF NOT EXISTS hotbackup;"
    "CREATE DATABASE IF NOT EXISTS log;"
    "CREATE DATABASE IF NOT EXISTS player;"
    "CREATE USER IF NOT EXISTS 'mt2'@'localhost' IDENTIFIED BY 'mt2';"
    "GRANT SELECT, INSERT, UPDATE, DELETE ON \`account\`.* TO 'mt2'@'localhost';"
    "GRANT SELECT, INSERT, UPDATE, DELETE ON \`common\`.* TO 'mt2'@'localhost';"
    "GRANT SELECT, INSERT, UPDATE, DELETE ON \`hotbackup\`.* TO 'mt2'@'localhost';"
    "GRANT SELECT, INSERT, UPDATE, DELETE ON \`log\`.* TO 'mt2'@'localhost';"
    "GRANT SELECT, INSERT, UPDATE, DELETE ON \`player\`.* TO 'mt2'@'localhost';"
    "account < /backup/account.sql"
    "anticheat < /backup/anticheat.sql"
    "common < /backup/common.sql"
    "log < /backup/log.sql"
    "player < /backup/player.sql"
)

# Execute the queries
for query in "${QUERIES[@]}"; do
    # mariadb -h "$MYSQL_HOST" -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" -e "$query"
    if [[ $query == *"<"* ]]; then
        # This is an import command
        filename=$(echo $query | cut -d' ' -f1)
        mariadb -h "$MYSQL_HOST" -u "root" -p"$MYSQL_ROOT_PASSWORD" "$filename" < "/backup/$filename.sql"
    else
        # This is a regular SQL command
        mariadb -h "$MYSQL_HOST" -u "root" -p"$MYSQL_ROOT_PASSWORD" -e "$query"
    fi

    if [ $? -ne 0 ]; then
        echo "Error executing query: $query"
        exit 1
    else
        echo "Query executed successfully: $query"
    fi
done

# Mark the script as executed
touch /tmp/migration_complete

echo "Migration script completed successfully."