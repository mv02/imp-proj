<!--
@file Dial.svelte
@brief Numeric dial component
@author Milan Vodák <xvodak07@stud.fit.vut.cz>
-->

<script lang="ts">
  let { label, value } = $props();
</script>

<div>
  <p class="text-4xl font-semibold">{value}</p>
  <p class="text-lg">{label}</p>
</div>
