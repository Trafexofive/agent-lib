#!/bin/bash
# Bootstrap script for Generic Automation Platform v1.0.0

echo "Creating CORRECT project structure..."

mkdir -p data
mkdir -p logs
mkdir -p gap_core
mkdir -p workspace
mkdir -p scripts

mkdir -p data/knowledge_sources
mkdir -p gap_core/core
mkdir -p gap_core/integrations/llm
mkdir -p workspace/orchestrations
mkdir -p workspace/agents/profiles
mkdir -p workspace/workflows

echo "Bootstrap complete."