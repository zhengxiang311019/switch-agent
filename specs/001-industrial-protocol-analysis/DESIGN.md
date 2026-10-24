# OT Analyzer Design System

**Status**: Proposed visual language for the future switch management UI. This repository has no application UI or style source to audit, so these are design targets, not scores of shipped screens.

## Design direction

Design for industrial network engineers who scan a dense overview, then move directly to a port, endpoint, relation, or event. Use a quiet graphite canvas, crisp table and chart surfaces, restrained cyan for navigation and selection, and semantic green/amber/red only for actual health states. Keep data confidence and capture completeness visible beside the metric they qualify.

Use a fixed desktop sidebar and top filter bar for fast troubleshooting. On narrow screens, collapse the sidebar and stack metric cards; keep dense relations in a horizontally scrollable table rather than shrinking text. Charts prioritize legible time axes and explicit units. A missing sample appears as a gap or “不可用”, never as zero.

## Reference patterns

- [Siemens SINEC NMS](https://sieportal.siemens.com/en-pk/products-services/10378781?tree=CatalogTree): long-term archives and diagnostic history motivate date-window controls and traceable trends.
- [Moxa MXview One](https://www.moxa.com/getmedia/ebdff140-74a4-4960-b5f0-d4fe32c2bac7/moxa-mxview-one-series-manual-v4.0.pdf): device health summaries and event highlights inform the overview hierarchy and severity colors.
- [Paessler PRTG network maps](https://www.paessler.com/monitoring/network/network-mapping-tool): configurable maps motivate keeping endpoint/relation navigation accessible, while this design keeps evidence tables and time trends ahead of a topology canvas.

These products are reference points for interaction patterns, not sources to copy. The OT Analyzer is a single-switch, resource-aware diagnostic view rather than a multi-site NMS.

## Color tokens

| Token | Value | Use |
|---|---|---|
| canvas | #10161B | Main application background |
| surface | #182128 | Navigation and primary panels |
| surface-raised | #202B34 | Hover, selected row, and elevated panel |
| border | #34434D | Dividers and control outlines |
| text-primary | #E8EFF2 | Main labels and values |
| text-secondary | #A5B4BC | Supporting labels and metadata |
| text-muted | #7E909A | Disabled or tertiary information |
| accent | #55C2D1 | Navigation, links, selected filters |
| healthy | #54C99A | Confirmed healthy state |
| warning | #F1BC62 | Degraded or inferred state |
| critical | #F07870 | Confirmed critical event |
| unknown | #9BA8B0 | Unavailable, unknown, or unclassified state |
| chart-grid | #2B3942 | Chart guides |

Color never carries state alone: pair semantic color with text, icon, or status label. Do not color an inferred timeout as a confirmed network drop.

## Typography

- Interface: Inter, Segoe UI, Microsoft YaHei, sans-serif.
- Numeric metrics: Inter or Segoe UI with tabular numerals.
- Headings: 22/28 px page title, 16/22 px panel title, 13/18 px section label.
- Body and controls: 13/20 px; dense tables may use 12/18 px with clear row spacing.
- Metric values: 24–30 px, medium weight; units remain visible in a smaller adjacent label.

## Spacing, shape, and elevation

- Spacing scale: 4, 8, 12, 16, 24, 32, 40 px.
- Control height: 36 px compact, 40 px standard; minimum interactive target 40×40 px in the dense desktop UI.
- Radius: 4 px for inputs/buttons, 6 px for panels, pill shape only for status chips.
- Borders carry most separation; use one subtle shadow for menus/popovers. Avoid glass effects and decorative gradients.
- Breakpoints: compact below 900 px; narrow below 640 px. At narrow widths, collapse navigation and stack summary cards.

## Components and states

- **Status chip**: label + semantic icon + color; include “推断”, “采集不完整” or “不可用” when relevant.
- **Metric card**: value, unit, time window and small trend; unavailable metrics show a reason instead of 0.
- **Relation table**: sticky header, sortable columns, clear selected row; row details expose evidence and confidence.
- **Filter bar**: time range, protocol, port, endpoint/relation; preserve filters when opening detail and returning.
- **Event list**: time, category, endpoint/relation, count, confidence and capture-health evidence.
- **Chart**: labeled axes and units, accessible series labels, gaps where samples are absent; do not interpolate across capture gaps.
- **Empty state**: distinguish “没有匹配流量”, “监测未启用” and “采集不完整”.
- **Loading/error state**: retain last update time and state whether data is stale or unavailable.
- **Keyboard/accessibility**: visible focus ring, keyboard-operable controls, semantic table headings, and WCAG AA text contrast target.

## 10-dimension design targets

No rendered product UI exists yet, so these are targets rather than an audit score.

| Dimension | Target / 10 | Design target |
|---|---:|---|
| Color consistency | 9 | Use semantic palette and neutral surfaces only |
| Typography hierarchy | 8 | Stable title, table, label, and metric sizes |
| Spacing rhythm | 9 | Use the defined 4 px based scale |
| Component consistency | 8 | Reuse metric cards, chips, filters, and tables |
| Responsive behavior | 8 | Collapse navigation and preserve readable data |
| Dark mode | 8 | Dark is primary; maintain semantic contrast |
| Animation | 8 | Short state transitions only; no ambient motion |
| Accessibility | 9 | Keyboard, labels, focus, and contrast targets |
| Information density | 9 | Dense but grouped by workflow and severity |
| Polish | 8 | Explicit freshness, empty, degraded, and unavailable states |

## AI-slop guardrails

- No purple-to-blue hero gradients, glass panels, or decorative glow.
- Do not round every surface; reserve pills for compact status labels.
- No animated dashboard numbers or continuous chart motion.
- Do not use generic device illustrations where port, endpoint, or relation data belongs.
- Never encode alarm state by color alone or present missing telemetry as healthy.

## Companion artifacts

- Tokens: [design-tokens.json](design-tokens.json)
- CSS variables: [design-tokens.css](design-tokens.css)
- Interactive preview: [design-preview.html](design-preview.html)
