from flask import Flask, request, jsonify
import os
import requests
import signal
import sys

app = Flask(__name__)

@app.route("/new-banner", methods=["GET"])
def new_banner():
    # Get the 'name' and 'data' parameters from the URL query string
    name = request.args.get("name")
    data = request.args.get("data")

    # Check if 'name' is missing and handle it
    if not name:
        return jsonify({"error": "Missing 'name' parameter"}), 400

    # Optionally handle missing 'data' (if needed)
    if not data:
        return jsonify({"error": "Missing 'data' parameter"}), 400

    # Proceed with making the request to the db-svc
    response = requests.get(
        url=f"http://db-svc/set",
        params={
            "k": "banner-" + name,
            "v": data
        },
        timeout=5
    )
    
    # Handle non-200 response from db-svc
    if response.status_code != 200:
        return jsonify({
            "error": "Couldn't save the banner",
            "dbcode": response.status_code
        }), 500
    
    return "", 200  # Return success with an empty response

# Function to gracefully handle termination signals
def terminate(signal, frame):
    print("Terminating")
    sys.exit(0)

if __name__ == "__main__":
    signal.signal(signal.SIGTERM, terminate)
    app.run(host="0.0.0.0", port=80)

