import aiohttp
import asyncio
import json

async def log(params: dict, context) -> dict:
    message = params.get("message", "")
    print(f"[WORKFLOW_LOG] {message}")
    return {"status": "SUCCESS", "result": message, "error": None, "logs": [message]}

async def http_request(params: dict, context) -> dict:
    url = params.get("url")
    method = params.get("method", "GET").upper()
    headers = params.get("headers", {})
    body = params.get("body")

    if not url:
        return {"status": "FAILURE", "result": None, "error": "URL is a required parameter.", "logs": []}

    logs = [f"Executing HTTP {method} to {url}"]
    try:
        if isinstance(body, dict):
            body = json.dumps(body)

        async with aiohttp.ClientSession() as session:
            async with session.request(method, url, headers=headers, data=body) as response:
                try:
                    response_text = await response.text()
                except UnicodeDecodeError:
                    response_text = (await response.read()).decode('latin-1')

                status_code = response.status
                logs.append(f"Response status: {status_code}")
                
                result = {
                    "status_code": status_code,
                    "headers": dict(response.headers),
                    "body": response_text
                }
                try:
                    result["json"] = json.loads(response_text)
                except (json.JSONDecodeError, TypeError):
                    result["json"] = None

                if response.ok:
                    return {"status": "SUCCESS", "result": result, "error": None, "logs": logs}
                else:
                    logs.append(f"Response body: {response_text[:500]}") # Log snippet of error body
                    return {"status": "FAILURE", "result": result, "error": f"HTTP Error: {status_code}", "logs": logs}
    except Exception as e:
        error_msg = f"HTTP request failed: {e}"
        logs.append(error_msg)
        return {"status": "FAILURE", "result": None, "error": error_msg, "logs": logs}

async def data_transform(params: dict, context) -> dict:
    import jmespath
    query = params.get("query")
    input_data = params.get("input")

    if query is None:
        return {"status": "FAILURE", "result": None, "error": "'query' is a required parameter.", "logs":[]}
    if input_data is None:
        return {"status": "FAILURE", "result": None, "error": "'input' is a required parameter.", "logs":[]}
    
    try:
        result = jmespath.search(query, input_data)
        return {"status": "SUCCESS", "result": result, "error": None, "logs": [f"JMESPath query '{query}' executed."]}
    except Exception as e:
        error_msg = f"JMESPath query failed: {e}"
        return {"status": "FAILURE", "result": None, "error": error_msg, "logs": [error_msg]}
