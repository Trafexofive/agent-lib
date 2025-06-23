# Specification: The Multi-Runtime `tools.yaml`

**GOLD ESSENCE:** This specification is the bedrock of the platform's modularity and sovereignty, allowing any executable process to be integrated as a first-class tool.

The `tools.yaml` file is a central manifest that defines all tools available to the platform, regardless of their implementation language. The Orchestration Engine's `ToolRegistry` parses this file to build its library of capabilities.

## Core Schema

Each entry in the `tools` list is a dictionary with the following keys:

*   `id` (string, required): A unique, machine-readable identifier for the tool (e.g., `git.clone`, `video.transcode`).
*   `description` (string, required): A detailed, natural language description of what the tool does, its purpose, and when an agent should use it. **This is critical for agent-based tool selection.**
*   `runtime` (string, required): Specifies the execution environment. Must be one of `python`, `shell`, or `binary`.
*   `handler` (string, required): The entry point for the tool's logic. Its meaning depends on the `runtime`:
    *   For `python`: The fully qualified import path to the `BaseTool` implementation class (e.g., `gap_core.integrations.plex.PlexScanTool`).
    *   For `shell`: The command string to be executed. Parameters will be injected as environment variables.
    *   For `binary`: The absolute path to the compiled executable within the container (e.g., `/app/bin/ffprobe`).
*   `parameters` (object, optional): An object defining the parameters the tool accepts. Each key is a parameter name.
    *   `description` (string, required): A clear explanation of the parameter for agents and humans.
    *   `type` (string, required): The expected data type (e.g., `string`, `integer`, `boolean`, `path`).
    *   `required` (boolean, optional): Whether the parameter is mandatory.
*   `returns` (string, optional): A natural language description of what the tool's execution returns (e.g., `A JSON object with video stream details`, `The stdout of the git command`).

## Example Entry

```yaml
- id: 'video.get_metadata'
  description: 'Uses the ffprobe binary to extract detailed technical metadata from a video or audio file.'
  runtime: 'binary'
  handler: '/app/bin/ffprobe'
  parameters:
    input_file:
      description: 'The full path to the input video file.'
      type: 'path'
      required: true
  # Note: The engine would construct the final command from the handler and parameters.
  # e.g., /app/bin/ffprobe -v quiet -print_format json -show_format "${PARAM_INPUT_FILE}"
  returns: 'A JSON string containing the ffprobe output.'
```