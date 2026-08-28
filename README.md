# Malta2TreeWriter

Allpix Squared module that writes simulated MALTA2 hit data to ROOT TTrees
in the same format as MaltaMultiDAQ, enabling direct online monitoring with
Corryvreckan's `EventLoaderMALTA`.

---

## Overview

For each detector, one ROOT file is created per run:

```
<output_dir>/run_<NNNNNN>_<plane>.root
```

Each file contains a TTree named `MALTA` with one entry per simulated pixel hit,
plus a `config/` directory holding the full Allpix configuration metadata.

The branch layout matches MaltaDAQ's `Malta2Tree` exactly so that
`EventLoaderMALTA` can read simulation and real-hardware data with the same config.

---

## Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `file_name` | string | `"malta2_sim_output"` | Basename (unused; file names are auto-generated as `run_NNNNNN_<plane>.root`) |
| `run_number` | int | `1` | Run number embedded in file names and the `runNumber` branch |
| `trigger_detectors` | array of 2 strings | none | If set, an event is written out (for **all** detectors, including the two listed here) only when both named detectors have at least one `PixelHit` in that event. Simulates a coincidence trigger built from two telescope arm planes gating the readout of the whole telescope. Disabled (every event written) if not set. |

---

## Output Branches

| Branch | Type | Description |
|---|---|---|
| `pixel` | `uint32_t` | Pixel bitmask (16 pixels per word) |
| `group` | `uint32_t` | Row group index |
| `parity` | `uint32_t` | Sub-group parity bit |
| `dcolumn` | `uint32_t` | Double-column index |
| `bcid` | `uint32_t` | Bunch crossing ID (25 ns resolution) |
| `winid` | `uint32_t` | Window ID within BCID (3.125 ns) |
| `phase` | `uint32_t` | Phase within window (0.39 ns) |
| `l1id` | `uint32_t` | 12-bit event counter (= `event->number`) |
| `l1idC` | `uint32_t` | 32-bit cumulative event counter (= `event->number`) — used by `EventLoaderMALTA` to avoid rollover |
| `runNumber` | `uint32_t` | Run number |
| `isDuplicate` | `uint32_t` | Always 0 for simulation |
| `timer` | `float` | Always 0 for simulation (real time from MaltaDAQ) |

> **`l1id` vs `l1idC`**: real hardware `l1id` wraps at 4096 every 4096 triggers.
> `EventLoaderMALTA` groups events by `l1idC`, which never wraps.
> Malta2TreeWriter sets both to `event->number` so simulation and real-hardware
> data are handled identically by the reader.

---

## Online Monitoring

Malta2TreeWriter calls `TTree::AutoSave("SaveSelf")` every ~1 second so that
a simultaneously running `EventLoaderMALTA` (with `online_mode = true`) can
read new entries without waiting for the simulation to finish.

**Important**: AutoSave is called **before** the early-return check for
events with no pixel hits.  Without this ordering, a quiet beam period
(many events with zero hits) would delay the flush by minutes.

```
run(event)
  ├─ [acquire ROOT lock]
  ├─ check 1-second AutoSave timer     ← always runs
  ├─ fetch PixelHitMessages
  │   └─ no hits → return early        ← AutoSave already done above
  ├─ trigger_detectors coincidence check (if configured)
  │   └─ not satisfied → return early  ← no tree gets any entry for this event
  └─ fill TTree branches
```

---

## Configuration Example

```ini
[Malta2TreeWriter]
file_name  = "malta2_sim_output"
run_number = 8
# Only write out events where MALTA2_0 and MALTA2_2 both have a hit:
trigger_detectors = "MALTA2_0", "MALTA2_2"
```

Corryvreckan side (`EventLoaderMALTA`):

```ini
[EventLoaderMALTA]
detector_names = "MALTA_0", "MALTA_1", "MALTA_2"
base_path      = "/path/to/allpix/output"
run_number     = 8
online_mode    = true
```

---

## See Also

- [EventLoaderMALTA](../../../corryvreckan/src/modules/EventLoaderMALTA/) — reader side
- [OnlineMonitor](../../../corryvreckan/src/modules/OnlineMonitor/) — real-time display
