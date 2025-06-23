import os
import httpx
import google.generativeai as genai
from groq import Groq
from pydantic import BaseModel
from typing import List, Dict, Any, Tuple, Optional

class Message(BaseModel):
    role: str
    content: str

class Usage(BaseModel):
    prompt_tokens: Optional[int] = None
    completion_tokens: Optional[int] = None
    total_tokens: Optional[int] = None

async def call_gemini_api(
    model_name: str, 
    messages: List[Message], 
    api_key: str,
    max_tokens: Optional[int] = None,
    temperature: Optional[float] = None
) -> Tuple[str, Usage]:
    genai.configure(api_key=api_key)
    
    gemini_messages = []
    system_instruction = None
    for msg in messages:
        if msg.role == "system":
            system_instruction = msg.content
            continue
        gemini_messages.append({'role': msg.role if msg.role != 'assistant' else 'model', 'parts': [{'text': msg.content}]})

    model = genai.GenerativeModel(
        model_name=model_name,
        system_instruction=system_instruction if system_instruction else None
    )
    
    generation_config_params = {}
    if max_tokens is not None:
        generation_config_params['max_output_tokens'] = max_tokens
    if temperature is not None:
        generation_config_params['temperature'] = temperature
    generation_config = genai.types.GenerationConfig(**generation_config_params) if generation_config_params else None

    try:
        response = await model.generate_content_async(
            contents=gemini_messages,
            generation_config=generation_config
        )
        
        response_text = ""
        if response.candidates and response.candidates[0].content and response.candidates[0].content.parts:
            for part in response.candidates[0].content.parts:
                if hasattr(part, 'text'):
                    response_text += part.text
        else:
            response_text = "No content returned from Gemini."
            if response.prompt_feedback and response.prompt_feedback.block_reason:
                response_text += f" (Blocked: {response.prompt_feedback.block_reason_message or response.prompt_feedback.block_reason})"
            elif response.candidates and response.candidates[0].finish_reason != genai.types.FinishReason.STOP:
                 response_text += f" (Finish reason: {response.candidates[0].finish_reason.name})"

        usage = Usage(prompt_tokens=None, completion_tokens=None, total_tokens=None) 
        return response_text, usage
    except Exception as e:
        print(f"Error calling Gemini API: {e}")
        raise e

async def call_groq_api(
    model_name: str, 
    messages: List[Message], 
    api_key: str,
    max_tokens: Optional[int] = None,
    temperature: Optional[float] = None
) -> Tuple[str, Usage]:
    client = Groq(api_key=api_key)
    
    groq_messages = [{'role': msg.role, 'content': msg.content} for msg in messages]
    
    chat_completion_params = {
        "messages": groq_messages,
        "model": model_name,
    }
    if max_tokens is not None:
        chat_completion_params['max_tokens'] = max_tokens
    if temperature is not None:
        chat_completion_params['temperature'] = temperature

    try:
        chat_completion = await client.chat.completions.create(**chat_completion_params) # Use await for async client if available, else sync
        
        response_content = ""
        if chat_completion.choices and chat_completion.choices[0].message:
            response_content = chat_completion.choices[0].message.content or ""
        
        usage_stats = Usage()
        if chat_completion.usage:
            usage_stats.prompt_tokens = chat_completion.usage.prompt_tokens
            usage_stats.completion_tokens = chat_completion.usage.completion_tokens
            usage_stats.total_tokens = chat_completion.usage.total_tokens
            
        return response_content, usage_stats
    except Exception as e:
        print(f"Error calling Groq API: {e}")
        if hasattr(e, 'response') and hasattr(e.response, 'text'):
             # Assuming e.response is similar to httpx.Response
            error_detail = e.response.text if isinstance(e.response.text, str) else str(e.response.content)
            raise Exception(f"Groq API Error: {e.response.status_code} - {error_detail}")
        raise e
