import logging
import random
import os
from typing import List

from app.models_platform import AgentCreate, SmartAgentCreateRequest

logger = logging.getLogger(__name__)

async def generate_smart_agent_config(request: SmartAgentCreateRequest) -> AgentCreate:
    logger.info(f"SMART AGENT CREATION (SIMULATED) for keywords: {request.primary_keywords}")
    base_name = "_" .join(kw.lower().replace(" ", "_").replace("-", "_") for kw in request.primary_keywords[:3])
    agent_name = f"{base_name}_reporter"
    if len(agent_name) > 90: agent_name = agent_name[:90]
    agent_name += f"_{random.randint(100,999)}"

    agent_description = f"Autonomous agent monitoring topics related to: {', '.join(request.primary_keywords)}."
    monitoring_targets = []
    joined_keywords = " ".join(request.primary_keywords).lower()

    if "quantum computing" in joined_keywords:
        monitoring_targets.extend([
            "https://phys.org/tags/quantum+computing/rss",
            "https://www.technologyreview.com/tag/quantum-computers/",
            "https://arxiv.org/list/quant-ph/recent"
        ])
    if "renewable energy" in joined_keywords:
        monitoring_targets.extend([
            "https://www.rechargenews.com/rss",
            "https://www.irena.org/news"
        ])
    if "ai ethics" in joined_keywords:
        monitoring_targets.extend([
            "https://www.eff.org/rss/updates.xml",
            "https://www.brookings.edu/topic/artificial-intelligence/feed/"
        ])
    
    keyword_query = " OR ".join(f'\"{kw}\"' for kw in request.primary_keywords)
    monitoring_targets.append(f"google-news-search:{keyword_query}")
    monitoring_targets.append(f"generic-web-search:{keyword_query}")
    
    monitoring_targets = sorted(list(set(monitoring_targets)))[:7]

    if not monitoring_targets and request.primary_keywords:
        monitoring_targets.append(f"broad-web-search:{request.primary_keywords[0]}")

    return AgentCreate(
        name=agent_name,
        description=agent_description,
        keywords=list(set(request.primary_keywords)),
        monitoring_targets=monitoring_targets,
        reporting_frequency="daily"
    )
