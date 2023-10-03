/*
 * Vencord, a modification for Discord's desktop app
 * Copyright (c) 2022 Vendicated and contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

let userscript;

const promise = new Promise((resolve) => {
  console.log("STARTING PROMISE")
  setTimeout(() => {
    new QWebChannel(qt.webChannelTransport, function (channel) {
      console.log("GOT CHANNEL")
      userscript = channel.objects.userscript;
      console.log(userscript)
      resolve();
    });
  });
});

async function prepareUserscript() {
  if (!userscript) await promise;
}


/// <reference path="../src/modules.d.ts" />
/// <reference path="../src/globals.d.ts" />

import * as DataStore from "../src/api/DataStore";
import { debounce } from "../src/utils";
import { getThemeInfo } from "../src/main/themes";

// Discord deletes this so need to store in variable
const { localStorage } = window;

// listeners for ipc.on
const cssListeners = new Set<(css: string) => void>();
const NOOP = () => { };
const NOOP_ASYNC = async () => { };

const themeStore = DataStore.createStore("VencordThemes", "VencordThemeData");

// probably should make this less cursed at some point
window.VencordNative = {
    themes: {
        uploadTheme: (fileName: string, fileData: string) => DataStore.set(fileName, fileData, themeStore),
        deleteTheme: (fileName: string) => DataStore.del(fileName, themeStore),
        getThemesDir: async () => "",
        getThemesList: () => DataStore.entries(themeStore).then(entries =>
            entries.map(([name, css]) => getThemeInfo(css, name.toString()))
        ),
        getThemeData: (fileName: string) => DataStore.get(fileName, themeStore),
        getSystemValues: async () => ({}),
    },

    native: {
        getVersions: () => ({}),
        openExternal: async (url) => {
          await prepareUserscript();
          userscript.openURL(url);
        }
    },

    updater: {
        getRepo: async () => ({ ok: true, value: "https://github.com/Vendicated/Vencord" }),
        getUpdates: async () => ({ ok: true, value: [] }),
        update: async () => ({ ok: true, value: false }),
        rebuild: async () => ({ ok: true, value: true }),
    },

    quickCss: {
        get: async () => {
          await prepareUserscript();
          return userscript.getQuickCSS();
        },
        set: NOOP_ASYNC,
        addChangeListener(cb) {
            cssListeners.add(cb);
        },
        addThemeChangeListener: NOOP,
        openFile: NOOP_ASYNC,
        async openEditor() {
            userscript.editQuickCSS();
        },
    },

    settings: {
        get: () => localStorage.getItem("VencordSettings") || "{}",
        set: async (s: string) => localStorage.setItem("VencordSettings", s),
        getSettingsDir: async () => "LocalStorage"
    },

    pluginHelpers: {} as any,
};
