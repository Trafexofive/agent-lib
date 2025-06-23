import logging
import asyncio
import datetime
import random
import uuid
from typing import List, Dict, Any

from app.models_platform import AgentDB

logger = logging.getLogger(__name__)

async def gather_information_for_agent(agent: AgentDB) -> List[Dict[str, Any]]:
    logger.info(f"INFORMATION GATHERING (SIMULATED) for agent: {agent.name} (ID: {agent.id})")
    logger.debug(f"Monitoring targets: {agent.monitoring_targets}")
    logger.debug(f"Keywords: {agent.keywords}")

    await asyncio.sleep(random.uniform(1, 3))

    findings = []
    for i, target in enumerate(agent.monitoring_targets):
        num_items_found = random.randint(0, 2)
        for j in range(num_items_found):
            chosen_keyword = random.choice(agent.keywords) if agent.keywords else "general topic"
            source_type = target.split(':')[0] if ':' in target else "website"
            findings.append({
                "source_target": target,
                "retrieved_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "title": f"Simulated Item {j+1} on '{chosen_keyword}' from {source_type}",
                "snippet": f"This is a simulated abstract for agent '{agent.name}'. It found something about '{chosen_keyword}' from target '{target}'. The actual gathering service would populate this with real extracted data, summarize it, and check relevance.",
                "link": f"http://simulated-source.com/article/{uuid.uuid4().hex[:8]}",
                "relevance_score": round(random.uniform(0.7, 0.98), 3)
            })
    
    logger.info(f"Simulated gathering found {len(findings)} items for agent {agent.name}.")
    return findings

async def compile_findings_into_report_content(agent_name: str, findings: List[Dict[str, Any]], data_window_start: datetime.datetime) -> Dict[str, Any]:
    report_content = {
        "agent_name": agent_name,
        "report_generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "data_gathered_since": data_window_start.isoformat(),
        "number_of_findings": len(findings),
        "findings_summary": f"This report includes {len(findings)} items. Key themes might involve {', '.join(list(set(f['title'].split(' ')[-3] for f in findings if ' ' in f['title']))[:3]) if findings else 'various topics'}.",
        "detailed_findings": sorted(findings, key=lambda x: x.get('relevance_score', 0), reverse=True)
    }
    return report_content
