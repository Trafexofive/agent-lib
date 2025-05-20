import sys
import json

def calculate(operation, num1, num2):
    if operation == "add":
        return num1 + num2
    elif operation == "subtract":
        return num1 - num2
    elif operation == "multiply":
        return num1 * num2
    elif operation == "divide":
        if num2 == 0:
            raise ValueError("Division by zero")
        return num1 / num2
    else:
        raise ValueError(f"Unknown operation: {operation}")

if __name__ == "__main__":
    try:
        # The script expects parameters as a single JSON string argument
        params_json = sys.argv[1]
        params = json.loads(params_json)

        operation = params["operation"]
        num1 = params["num1"]
        num2 = params["num2"]

        result = calculate(operation, num1, num2)
        print(result)

    except IndexError:
        print("Error: No JSON parameters provided.", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print("Error: Invalid JSON parameters provided.", file=sys.stderr)
        sys.exit(1)
    except KeyError as e:
        print(f"Error: Missing parameter {e}.", file=sys.stderr)
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {e}.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}", file=sys.stderr)
        sys.exit(1)
