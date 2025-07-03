# Expedited Internet Bypass Protocol (EIBP)

EIBP is a clean-slate, scalable routing protocol designed to overcome limitations in traditional internet routing protocols like OSPF and BGP. It operates at Layer 2.5, in parallel with IP, enabling faster routing, auto-addressing, and seamless inter-AS communication.

## What is EIBP?

EIBP (Expedited Internet Bypass Protocol) introduces a modular and intelligent routing scheme that avoids flooding, simplifies configuration, and supports fault-tolerant communication across autonomous systems (AS). 

## How EIBP Works

- **Modular Architecture**: Networks are divided into three router tiers:
  - Core Routers (CR)
  - Distribution Routers (DR)
  - Access Routers (AR)

- **Auto-addressing**: Unique, reusable router addresses are assigned based on network position, eliminating manual IP configuration.

- **Routing without Flooding**: Packets are routed by comparing destination addresses to the router's own and its neighbors’ labels.

- **Intra- and Inter-AS Routing**:
  - Intra-AS routing happens without flooding.
  - Inter-AS communication is handled via Border Routers (BR), which maintain maps of neighbor AS IPs.

- **Fast Failure Recovery**: Hello messages are used for quick detection of router failures and automatic path re-routing.

## Key Features

- Operates independently from IP (Layer 2.5)
- Efficient packet forwarding without flooding
- Scalable, modular router hierarchy
- Quick failure detection and recovery
- Auto-addressing of routers
- Simplified inter-AS communication
- Backward-compatible with existing IP networks

## Project Context

This project was part of a research initiative under the guidance of **Dr. Nirmala Shenoy**, exploring scalable alternatives to traditional routing protocols for future internet architectures.
