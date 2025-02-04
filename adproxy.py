from flask import Flask, request, jsonify
import os
import requests
import signal
import logging

# Set up logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Get the ad server IP from environment variable
AD_SERVER_IP = os.environ.get("AD_SERVER_IP")
if not AD_SERVER_IP:
    logger.error("AD_SERVER_IP environment variable is not set")
    exit(1)

# Flask app
app = Flask(__name__)

@app.route("/", defaults={"path": ""})
@app.route("/<path:path>", methods=["GET", "POST"])
def to_adserver(path):
    """
    Forward requests to the ad server.
    """
    try:
        # Construct the target URL
        target_url = f"http://{AD_SERVER_IP}/{path}"
        logger.info(f"Forwarding to {target_url}")

        # Forward the request to the ad server
        response = requests.request(
            method=request.method,
            url=target_url,
            headers=request.headers,
            params=request.args,
            data=request.get_data(),
            timeout=5
        )
        # Return the response from the ad server
        return response.content, response.status_code, response.headers.items()
    except Exception as e:
        logger.error(f"Error during forward: {e}")
        return jsonify({"error": str(e)}), 500

def terminate(signal, frame):
    """
    Handle termination signals.
    """
    logger.info("Terminating")
    exit(0)

if __name__ == "__main__":
    # Handle termination signals
    signal.signal(signal.SIGTERM, terminate)
    signal.signal(signal.SIGINT, terminate)

    # Start the Flask app
    app.run(host="0.0.0.0", port=80)
