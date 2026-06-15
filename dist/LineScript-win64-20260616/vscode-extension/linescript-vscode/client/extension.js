"use strict";

const path = require("path");
const vscode = require("vscode");
const { LanguageClient, TransportKind } = require("vscode-languageclient/node");

let client;

function buildClient(context) {
  const serverModule = context.asAbsolutePath(path.join("server", "server.js"));
  const serverOptions = {
    run: { module: serverModule, transport: TransportKind.ipc },
    debug: {
      module: serverModule,
      transport: TransportKind.ipc,
      options: { execArgv: ["--nolazy", "--inspect=6010"] }
    }
  };

  const clientOptions = {
    documentSelector: [
      { scheme: "file", language: "linescript" },
      { scheme: "untitled", language: "linescript" }
    ],
    synchronize: {
      configurationSection: "linescript",
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.{lsc,ls}")
    }
  };

  return new LanguageClient(
    "linescriptLanguageServer",
    "LineScript Language Server",
    serverOptions,
    clientOptions
  );
}

async function restartLanguageServer(context) {
  if (client) {
    await client.stop();
  }
  client = buildClient(context);
  context.subscriptions.push(client.start());
  vscode.window.showInformationMessage("LineScript language server restarted.");
}

function activate(context) {
  client = buildClient(context);
  context.subscriptions.push(client.start());

  const restartDisposable = vscode.commands.registerCommand("linescript.restartLanguageServer", async () => {
    try {
      await restartLanguageServer(context);
    } catch (err) {
      const msg = err && err.message ? err.message : String(err);
      vscode.window.showErrorMessage("Failed to restart LineScript language server: " + msg);
    }
  });
  context.subscriptions.push(restartDisposable);
}

async function deactivate() {
  if (!client) return undefined;
  return client.stop();
}

module.exports = { activate, deactivate };
