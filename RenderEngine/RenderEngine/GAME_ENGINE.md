# RIFT Engine – z editora na game engine

Prvý krok prestavby „mini Blendera“ na engine v štýle Roblox Studia. Doterajší editor (vkladanie tvarov, gizmo, textúry, hierarchia, undo, ukladanie/načítanie) zostáva **nástrojom na stavanie scény**. Pribudol herný režim, fyzika, skriptovanie a Discord Rich Presence.

## Čo pribudlo

| Funkcia | Kde | Use case |
|---|---|---|
| **Play / Stop** (F5, Esc ukončí hru) – scéna sa po Stop vráti presne do stavu pred Play | Toolbar, menu *Game* | UC-017 |
| **Hráčska postava** – WASD/šípky, Space skok, Shift šprint, pravé tlačidlo myši = rozhliadanie, kamera sleduje hráča | `GameRuntime` | UC-018 |
| **Fyzika** – *Anchored* (nehýbe sa), *Can Collide*, *Bounciness*, gravitácia, kolízie AABB, „kill plane“ | Properties → *Game* | UC-016 |
| **Skripty (RiftScript)** – `on start / on update / on touch`, premenné, if/elseif/else, repeat | Properties → *Game* → *Add Script* | UC-014, UC-015 |
| **Script Editor** – kontrola syntaxe počas písania, referencia jazyka | menu *Game → Script Editor* | UC-015 |
| **Output** – výpisy `print` a chyby skriptov s číslom riadku | menu *Game → Output* | – |
| **Insert menu** – Part, Sphere, Wedge, Baseplate, Spawn Point a hotové skriptované šablóny: Kill Brick, Spinner, Coin, Jump Pad, Falling Block, Finish Line | menu *Insert* | UC-001 |
| **Discord Rich Presence** – „Building house.json · 12 objects“ / „Playtesting …“ + čas | `RichPresence` | – |

Panely *Object List* a *Scene Controls* sa teraz volajú **Explorer** a **Toolbar** (rozloženie okien v `imgui.ini` zostalo).
Nové vlastnosti objektov sa ukladajú do scény pod kľúčom `"game"`; staré scény sa načítajú bez zmeny (všetko Anchored).

## Súbory

- `RiftScript.h/.cpp` – jazyk: tokenizer, parser (AST), interpreter. Nevie nič o engine; príkazy a polia mu dodáva hostiteľ.
- `GameRuntime.h/.cpp` – herný režim: snapshot/obnova scény, hráč, fyzika, udalosti `touch`, implementácia príkazov skriptov.
- `InterfaceGame.cpp` – UI pre herné časti (Play, Insert, Game vlastnosti, Script Editor, Output, overlay správ).
- `RichPresence.h/.cpp` – Discord Game SDK (už bol v projekte v `libraries/discord-rpc`).
- `assets/branding/` – návrhy loga (SVG) + `preview.html`.
- Zmenené: `Object.h/.cpp` (`GameProps`), `Engine.h/.cpp`, `Interface.h/.cpp`, `Settings.h`, `RenderEngine.vcxproj` (+ `imgui/misc/cpp/imgui_stdlib.cpp`).
- Záloha pôvodných zdrojákov: `../_backup_pred_enginom/`.

## Formát scény v2 (malé JSON súbory)

- Základné tvary sa ukladajú menom (`"mesh": "sphere"`) namiesto tisícok vrcholov, ostatné siete ako jedno ploché pole `"v"` (8 čísel na vrchol).
- Ukladajú sa len hodnoty odlišné od predvolených, čísla sú zaokrúhlené na 5 desatinných miest, jeden objekt = jeden riadok.
- Výsledok: všetky scény v `assets/scenes` spolu **35 MB → ~7 KB** (napr. `mickey.json` 12 MB → 1,2 KB). Staré súbory (v1) sa stále načítajú a pri uložení sa prevedú.
- Scénu možno otvoriť z príkazového riadku: `RenderEngine.exe assets/scenes/demo_obby.json`.
- `assets/scenes/demo_obby.json` – ukážková hra (mince, spinner, kill brick, jump pad, cieľ).

## RiftScript – príklad

```lua
var speed = 90

on start
    print "Ahoj z " .. self.name
end

on update
    rotate 0, speed * dt, 0
    if key("E") then
        move 0, 2 * dt, 0
    end
end

on touch
    if other == "Player" then
        color 1, 0, 0
        message "Dotkol si sa ma!", 2
    end
end
```

Úplný zoznam príkazov a funkcií je v Script Editore (*Language reference*) a v `rs::languageReference()`.
Ochrana: jedna udalosť smie vykonať max. 200 000 príkazov, takže nekonečná slučka hru nezamrazí – skript sa zastaví s chybou.

## Discord Rich Presence – nastavenie (jednorazovo)

1. <https://discord.com/developers/applications> → *New Application* → názov **RIFT Engine** (to Discord zobrazí ako „Playing RIFT Engine“).
2. *Rich Presence → Art Assets* → nahraj logo s kľúčom **`rift_logo`** (min. 512×512 PNG).
3. *Application ID* skopíruj do `RichPresence::kClientId` v `RichPresence.h`, alebo do `settings.json` ako `"discordClientId"`.
4. Vypnutie: `"discordRichPresence": false` v `settings.json`.

Bez ID, alebo keď Discord nebeží, sa nič nedeje (engine beží normálne; Discord sa skúša pripojiť každých 15 s).

## Obmedzenia / ďalšie kroky

- Fyzika je zámerne jednoduchá: statické časti sa testujú ako orientované kvádre (SAT), pohyblivé telesá a hráč ako osovo zarovnané boxy, simulujú sa len objekty bez rodiča a otáčajúca sa plošina hráča neunáša. Pre plnú fyziku → Jolt/Bullet.
- RiftScript nemá funkcie ani zoznamy a nevie vytvárať nové objekty. Ak by ste chceli plnohodnotný jazyk, ďalší krok je **Lua** (`sol2` + Lua 5.4) – rozhranie `rs::Host` je pripravené tak, aby sa dal interpreter vymeniť.
- Chýba ešte z use case-ov: UC-006 import modelu (.obj), UC-008 svetlá, UC-011 editor shaderov, UC-019 export hry (samostatný „player“ .exe, ktorý načíta scénu a hneď spustí Play).
- Undo nepokrýva zmeny Game vlastností a textu skriptu.

## Build

Projekt má nastavený toolset **v145** (VS 2026). Vo VS 2022 buď *Retarget solution*, alebo:

```
MSBuild RenderEngine.sln /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143
```
