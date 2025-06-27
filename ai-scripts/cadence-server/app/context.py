import jmespath
import re
from typing import Any, Dict, List, Optional

class WorkflowContext:
    """Manages the state and variable resolution for a single workflow run."""
    def __init__(self, initial_context_data: Dict[str, Any]):
        self.data = initial_context_data

    def get(self, path: str, default: Any = None) -> Any:
        """Retrieves a value from the context using JMESPath syntax."""
        try:
            return jmespath.search(path, self.data)
        except Exception as e:
            print(f"[CONTEXT_ERROR] Could not get path '{path}': {e}")
            return default

    def set(self, path: str, value: Any):
        """Sets a value in the context using dot notation."""
        keys = path.split('.')
        d = self.data
        for key in keys[:-1]:
            d = d.setdefault(key, {})
        d[keys[-1]] = value

    def _resolve_value(self, value: Any, extra_context: Optional[Dict[str, Any]]) -> Any:
        """Resolves a single value, handling strings, dicts, and lists."""
        if isinstance(value, str):
            return self.resolve_string(value, extra_context)
        elif isinstance(value, dict):
            return self.resolve_dict(value, extra_context)
        elif isinstance(value, list):
            return [self._resolve_value(item, extra_context) for item in value]
        return value

    def resolve_string(self, input_str: str, extra_context: Optional[Dict[str, Any]] = None) -> str:
        """Resolves all $(...) expressions in a string."""
        if not isinstance(input_str, str) or '$(' not in input_str:
            return input_str

        full_context_data = {**self.data, **(extra_context or {})}

        def replacer(match):
            path = match.group(1).strip()
            
            # Handle simple filters like | to_json or | length
            if ' | to_json' in path:
                path = path.replace(' | to_json', '').strip()
                value = jmespath.search(path, full_context_data)
                return json.dumps(value) if value is not None else 'null'
            
            if ' | length' in path:
                path = path.replace(' | length', '').strip()
                value = jmespath.search(path, full_context_data)
                return str(len(value)) if hasattr(value, '__len__') else '0'

            value = jmespath.search(path, full_context_data)
            
            if isinstance(value, (dict, list)):
                return json.dumps(value)
            
            return str(value) if value is not None else ''

        # Use a loop to handle nested resolutions
        resolved_str = input_str
        for _ in range(5): # Limit recursion depth to prevent infinite loops
            new_str = re.sub(r'\$\((.*?)\)', replacer, resolved_str)
            if new_str == resolved_str:
                break
            resolved_str = new_str
        return resolved_str

    def resolve_dict(self, data_dict: Dict[str, Any], extra_context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Recursively resolves all string values in a dictionary."""
        resolved_dict = {}
        for key, value in data_dict.items():
            resolved_dict[key] = self._resolve_value(value, extra_context)
        return resolved_dict
