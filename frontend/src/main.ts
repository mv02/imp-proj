/**
 * @file main.ts
 * @brief Svelte app initialization
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

import { mount } from "svelte";
import "./app.css";
import App from "./App.svelte";

const app = mount(App, {
  target: document.getElementById("app")!,
});

export default app;
