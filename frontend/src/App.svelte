<!--
@file App.svelte
@brief Metronome controls page
@author Milan Vodák <xvodak07@stud.fit.vut.cz>
-->

<script lang="ts">
  import "./app.css";
  import { Button, ButtonGroup, Range } from "flowbite-svelte";
  import Dial from "./components/Dial.svelte";

  let bpm = 120;
  let volume = 50;
  let beats = 4;

  let ws = new WebSocket("http://metronome.local/ws");
  ws.onopen = () => console.log("WS connected");
  ws.onclose = () => console.log("WS disconnected");

  $: if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(`bpm:${bpm}`);
  }

  $: if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(`vol:${volume}`);
  }

  $: if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(`bts:${beats}`);
  }

  const volumeInc = (n: number) => (bpm = Math.min(250, bpm + n));
  const volumeDec = (n: number) => (bpm = Math.max(50, bpm - n));
</script>

<div class="grid grid-cols-2 grid-rows-2 items-end gap-8">
  <div id="bpm-control" class="control col-span-2">
    <Dial label="BPM" value={bpm} />
    <Range size="lg" min="50" max="250" bind:value={bpm} />

    <div class="flex justify-between">
      <ButtonGroup>
        <Button on:click={() => volumeDec(10)}>-10</Button>
        <Button on:click={() => volumeDec(1)}>-1</Button>
      </ButtonGroup>
      <ButtonGroup>
        <Button on:click={() => volumeInc(1)}>+1</Button>
        <Button on:click={() => volumeInc(10)}>+10</Button>
      </ButtonGroup>
    </div>
  </div>

  <div id="volume-control" class="control">
    <Dial label="Volume" value={volume} />
    <Range bind:value={volume} />
  </div>

  <div id="beats-control" class="control">
    <Dial label="Beats" value={beats} />
    <Range min="1" max="8" bind:value={beats} />
  </div>
</div>
