<!--
@file App.svelte
@brief Metronome controls page
@author Milan Vodák <xvodak07@stud.fit.vut.cz>
-->

<script lang="ts">
  import "./app.css";
  import metronomeImage from "./assets/metronome.svg";
  import { Button, ButtonGroup, Range } from "flowbite-svelte";
  import { PlaySolid, PauseSolid } from "flowbite-svelte-icons";
  import Dial from "./components/Dial.svelte";

  let running = true;
  let bpm: number;
  let volume: number;
  let beats: number;
  let periodMs: number;

  let ws = new WebSocket("http://metronome.local/ws");
  ws.onopen = () => {
    wsOpen = true;
    console.log("WS connected");
  };
  ws.onclose = () => {
    wsOpen = false;
    console.log("WS disconnected");
  };

  ws.onmessage = (e) => {
    let data = JSON.parse(e.data);
    console.log(data);
    ({ bpm, volume, beats, periodMs } = data);
  };

  let wsOpen = false;

  $: if (wsOpen && bpm) {
    ws.send(`bpm:${bpm}`);
  }

  $: if (wsOpen && volume) {
    ws.send(`vol:${volume}`);
  }

  $: if (wsOpen && beats) {
    ws.send(`bts:${beats}`);
  }

  const volumeInc = (n: number) => (bpm = Math.min(250, bpm + n));
  const volumeDec = (n: number) => (bpm = Math.max(50, bpm - n));

  function toggleMetronome() {
    fetch(`http://metronome.local/${running ? "stop" : "start"}`, {
      method: "PATCH",
    }).then((resp) => {
      console.log(resp);
      if (resp.status === 200) running = !running;
    });
  }
</script>

<div class="grid gap-8">
  <div id="image" class="col-span-2 flex justify-center">
    <img src={metronomeImage} alt="Metronome" class="w-24" />
  </div>

  <div id="bpm-control" class="control col-span-2">
    <Dial label="BPM" value={bpm} />
    <Range size="lg" min="50" max="250" disabled={!wsOpen} bind:value={bpm} />

    <div class="flex justify-between">
      <ButtonGroup>
        <Button disabled={!wsOpen} on:click={() => volumeDec(10)}>-10</Button>
        <Button disabled={!wsOpen} on:click={() => volumeDec(1)}>-1</Button>
      </ButtonGroup>

      <Button
        pill
        color="alternative"
        disabled={!wsOpen}
        on:click={toggleMetronome}
      >
        {#if running}
          <PauseSolid />
        {:else}
          <PlaySolid />
        {/if}
      </Button>

      <ButtonGroup>
        <Button disabled={!wsOpen} on:click={() => volumeInc(1)}>+1</Button>
        <Button disabled={!wsOpen} on:click={() => volumeInc(10)}>+10</Button>
      </ButtonGroup>
    </div>
  </div>

  <div id="volume-control" class="control">
    <Dial label="Volume" value={volume} />
    <Range disabled={!wsOpen} bind:value={volume} />
  </div>

  <div id="beats-control" class="control">
    <Dial label="Beats" value={beats} />
    <Range min="1" max="8" disabled={!wsOpen} bind:value={beats} />
  </div>
</div>
