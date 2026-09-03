# Split Files

Split files are stores as well-formed JSON and **must** contain one main object.

You can use splits located in [the resource repository](https://github.com/LibreSplit/LibreSplit-resources/tree/main/splits) to start creating your own split files and place them however you want.

## Main Object

| Key                 | Type               | Value                                               |
| ------------------- | ------------------ | --------------------------------------------------- |
| `title`             | string             | Title string at top of window                       |
| `attempt_count`     | int                | Number of attempts                                  |
| `comparison_method` | int                | Method of comparison between real_time or game_time |
| `start_delay`       | string (timestamp) | Non-negative delay until timer starts               |
| `world_record`      | time               | Best known [time](#time-object)                     |
| `splits`            | array              | Array of [split objects](#split-object)             |
| `theme`             | string             | Window theme                                        |
| `theme_variant`     | string             | Window theme variant                                |
| `width`             | int                | Window width                                        |
| `height`            | int                | Window height                                       |
| `components`        | array              | Advanced configuration of timer components          |

Most of the above keys are optional.
`comparison_method` determines which time is authoritative for determining things like PBs and best splits.
0 = real_time, 1 = game_time.

## Split Object

| Key            | Type   | Value                                                     |
| -------------- | ------ | --------------------------------------------------------- |
| `title`        | string | Split title                                               |
| `icon`         | string | Icon file path or url                                     |
| `time`         | time   | Split time - the total time up to this segment in your PB |
| `best_time`    | time   | Your best split time                                      |
| `best_segment` | time   | Your best segment time (split gold)                       |

## Time Object

| Key         | Type   | Value                                                             |
| ----------- | ------ | ----------------------------------------------------------------- |
| `real_time` | string | Real time elapsed for your run                                    |
| `game_time` | string | Game time elapsed for your run as controlled by your autosplitter |

A time object is an object containing times for real time and game time. Times in a time object are strings in `HH:MM:SS.mmmmmm` format.

Icons can be either a local file path (preferably absolute) or a URL. Note that only GTK-supported image formats will work. For example, `.svg` and `.webp` won't.

## Example

Here is a quick example of how a simple split file would look:

```json
{
  "title": "School - Homework%",
  "attempt_count": 55,
  "comparison_method": 1,
  "splits": [
    {
      "title": "Maths",
      "time": {
        "real_time": "05:17:58.000000",
        "game_time": "05:12:55.000000"
      },
      "best_time": {
        "real_time": "04:14:38.000000",
        "game_time": "04:10:50.000000"
      },
      "best_segment": {
        "real_time": "04:14:38.000000",
        "game_time": "04:10:50.000000"
      }
    },
    {
      "title": "Science",
      "time": {
        "real_time": "07:45:10.000000",
        "game_time": "07:36:30.000000"
      },
      "best_time": {
        "real_time": "05:32:25.000000",
        "game_time": "05:26:25.000000"
      },
      "best_segment": {
        "real_time": "01:17:47.000000",
        "game_time": "01:15:35.000000"
      }
    }
  ],
  "width": 250,
  "height": 500
}
```

In the example above, the individual segment time for the "Maths" split would be `05:17:58.000000`, `05:12:55.000000` for real_time and game_time respectively
and the individual segment time for the "Science" split would be `02:27:12.000000`, `02:23:35.000000` for real_time and game_time respectively.
For the sake of the example, we assume the time it took between changing subjects is not timed in this hypothetical game's rules hence game_time is faster.
That difference is the difference between game_time and real_time as defined by the autosplitter.

game_time is controlled by the autosplitter. When an autosplitter does not define its own timing method, then game_time will mirror real_time - loading_time. The loading time is a state
that can be defined by an autosplitter. If none is provided then real_time - loading_time would just be real_time and thus game_time would be equivalent to real_time. This is for cases where
a game has no autosplitter at all for example.

Since this hypothetical example uses a `comparison_method` of game_time, it is possible for the `real_time` section of any of the splits to be slower than a prior
`real_time` value, since only `game_time` needs to be faster for you to record a gold or a PB.

## Advanced Configuration
Many of the GUI elements displayed on the timer -- **components** -- have configurable options.

### Hiding Components
If the `components` array is specified in the splits file, only those listed will be visible.

```json
{
  "title": "Lunch - Seven Cartons of Milk%",
  "components": [
    "title",
    "splits",
    "timer"
  ],
  "splits": [
  ], // ...
```

In the above configuration, only the run title, splits, and a timer will be shown. Previous segment, sum of best, etc., will not be shown.

### Changing the Order
Perhaps you would prefer the detailed timer is on top of the splits, this is also supported! The components are parsed in the order they are given in the array.

```json
{
  "title": "Lunch - Seven Cartons of Milk%",
  "components": [
    "timer",
    "pb",
    "wr",
    "splits"
  ],
  "splits": [
  ], // ...
```

This configuration would create a window where the main timer is at the very top, followed by your personal best, the world record (if set), and the splits (as defined in the `splits` portion of this file.)

### Component Configuration
Valid types for the components are strings, shown above, or nested JSON objects. If a component is defined with a JSON object, it must contain, at the minimum, the component's name under the key `component`:

```json
{
  "component": "prev-segment"
}
```

Within this JSON object, any supported configuration options may be provided. For any that are absent, the default will value will be used. Thus, a component configuration object need never be exhaustive.

```json
{
  "title": "Skyrim - Game Crash%",
  "components": [
    "title",
    {
      "component": "splits",
      "height": 5
    },
    {
      "component": "info-box",
      "class": "health",
      "left": "Health"
      "right": ":PlayerHealth"
    },
    {
      "component": "info-box",
      "class": "mana",
      "left": "Mana"
      "right": ":ManaLevel"
    }
  ],
  "splits": [
  ], // ...
```

This configuration creates a timer with the normal run title, splits with a maximum of 5 splits shown, and two variable-information text boxes, each tracking a value set in the auto-splitter Lua runtime (see [[../auto-splitters.md]].) Notice the multiplicity of the "text-box" element: Libresplit supports multiple instances of any component.

### Component Options Compendium

| Component      | Option        | Type                            | Description                                                                                                                   |
|----------------|---------------|---------------------------------|-------------------------------------------------------------------------------------------------------------------------------|
| title          |               |                                 | A static, centered text object that displays the title of the current run category.                                           |
|                | source        | str - `:luavar`                 | The origin of the text to be displayed. Always the title in the splits file unless a specified :luavar is a string value.     |
|                | simple        | bool                            | If set (true) display only the title (not the run number.)                                                                    |
| splits         |               |                                 | A tabular display of the levels that comprise a run.                                                                          |
| -> (WIP)       | height        | int                             | The maximum number of splits to be displayed at a time.                                                                       |
| -> (WIP)       | pin-final     | bool                            | Always display the final split.                                                                                               |
| -> (WIP)       | reverse       | bool                            | If set (true) then the splits will be ordered from bottom to top.                                                             |
| timer          |               |                                 | The current run time.                                                                                                         |
|                | simple        | bool                            | If set (true) then only a basic timer will be displayed (no additional information.)                                          |
|                | source        | str - `real`, `game`, `:luavar` | Overrides the global timing method for this run. I.E., allows for a run that uses game time but requires a real time display. |
| prev-segment   |               |                                 | Shows timing information about the most recently completed level during a run.                                                |
| best-sum       |               |                                 | Displays the possible run time when composing the sum of each best segment time.                                              |
| pb             |               |                                 | Displays the fastest "personal best" time for this run category.                                                              |
| wr             |               |                                 | Displays the fastest "world record" time for this run category.                                                               |
