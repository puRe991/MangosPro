-- Help articles shown in the Customer Support window.
-- category = index into MangosProSupport_Categories, top = listed under "Top Issues",
-- hot / new = the fire / exclamation icons in the list.

local H = "|cffffd100";  -- heading colour
local C = "|cff00ccff";  -- command colour
local R = "|r";

MangosProSupport_Motd = "Willkommen auf MangosPro - TBC 2.4.3 mit Playerbots, die Raids wirklich spielen.";
MangosProSupport_ServerNotice = nil;

MangosProSupport_Categories = {
	"Erste Schritte",
	"Bots",
	"Raids",
	"Befehle",
	"Probleme & Tickets",
};

MangosProSupport_Articles = {
	{
		id = 1, category = 1, top = true, hot = true,
		title = "Willkommen auf MangosPro",
		keywords = "start anfang server repack github info",
		text = H .. "Was ist MangosPro?" .. R .. "\n" ..
			"MangosPro ist ein TBC-Server (2.4.3) auf Basis von CMaNGOS mit fest eingebauten Playerbots. " ..
			"Ziel des Projekts: Bots, die Dungeons und Raids von Karazhan bis Sunwell sinnvoll mitspielen.\n\n" ..
			H .. "Wo finde ich Updates?" .. R .. "\n" ..
			"Neue Versionen, Änderungen und den Quellcode gibt es kostenlos auf\n" ..
			C .. "github.com/puRe991/MangosPro" .. R .. "\n\n" ..
			H .. "Erste Schritte" .. R .. "\n" ..
			"- Hol dir Bots in die Gruppe (Artikel \"Bots in die Gruppe holen\").\n" ..
			"- Mit " .. C .. ".help" .. R .. " und " .. C .. ".commands" .. R .. " siehst du alle Befehle, die du benutzen darfst.\n" ..
			"- Im Suchfeld oben kannst du nach Stichworten wie \"lust\", \"raid\" oder \"stuck\" suchen.",
	},
	{
		id = 2, category = 2, top = true, hot = true,
		title = "Bots in die Gruppe holen",
		keywords = "bot add invite einladen gruppe party remove entfernen group",
		text = H .. "Eigene Charaktere als Bot" .. R .. "\n" ..
			"Jeder Charakter deines Accounts kann als Bot mitspielen:\n" ..
			C .. ".bot add NAME" .. R .. " - Charakter einloggen und in die Gruppe holen\n" ..
			C .. ".bot remove NAME" .. R .. " - Bot wieder ausloggen\n\n" ..
			H .. "Andere Bots" .. R .. "\n" ..
			"Bots, die in der Welt unterwegs sind, kannst du ganz normal mit " .. C .. "/invite NAME" .. R ..
			" einladen - sie nehmen die Einladung automatisch an.\n" ..
			C .. ".bot group" .. R .. " erstellt vier Bots mit passenden Klassen auf deiner Stufe (sofern der Server es erlaubt).\n\n" ..
			H .. "Tipp" .. R .. "\n" ..
			"Flüstere einem Bot " .. C .. "help" .. R .. ", dann listet er alle Befehle auf, die er versteht.",
	},
	{
		id = 3, category = 2, top = true,
		title = "Die wichtigsten Bot-Befehle",
		keywords = "follow folgen stay bleiben attack angriff reset befehl whisper flüstern",
		text = "Befehle kannst du einem Bot flüstern (" .. C .. "/w NAME befehl" .. R .. ") oder in den Gruppen-/Schlachtzugschat " ..
			"schreiben - dann gehorchen alle Bots der Gruppe.\n\n" ..
			C .. "follow" .. R .. " - dir folgen (belebt tote Bots wieder, holt weit entfernte Bots heran)\n" ..
			C .. "follow far" .. R .. " / " .. C .. "follow near" .. R .. " - Folgeabstand größer / kleiner\n" ..
			C .. "stay" .. R .. " - an der aktuellen Stelle bleiben\n" ..
			C .. "attack" .. R .. " - dein aktuelles Ziel angreifen\n" ..
			C .. "pull" .. R .. " - Tank-Bot pullt dein Ziel\n" ..
			C .. "neutralize" .. R .. " - dein Ziel kontrollieren (Verwandlung, Fesseln, Winterschlaf ...)\n" ..
			C .. "cast ZAUBER" .. R .. " - einen bestimmten Zauber wirken\n" ..
			C .. "reset" .. R .. " - Zustand, Befehle und Lootliste zurücksetzen\n" ..
			C .. "stats" .. R .. " - Gold, freie Taschenplätze und Reparaturkosten\n" ..
			C .. "repair all" .. R .. " - zum Rüstungsschmied laufen und alles reparieren\n" ..
			C .. "sell all" .. R .. " - unnötige graue/weiße Gegenstände verkaufen",
	},
	{
		id = 4, category = 3, top = true, hot = true,
		title = "Raids mit Bots",
		keywords = "raid karazhan gruul magtheridon ssc tk hyjal bt black temple zul'aman sunwell taktik strategie boss",
		text = H .. "Boss-Taktiken" .. R .. "\n" ..
			"Für alle TBC-Raids haben die Bots eigene Boss-Taktiken: Karazhan, Gruul, Magtheridon, " ..
			"Höhle des Schlangenschreins, Festung der Stürme, Hyjal, Schwarzer Tempel, Zul'Aman und Sunwell.\n" ..
			"Die Raid-Strategie wird im Schlachtzug automatisch aktiv - du musst nichts einschalten.\n\n" ..
			H .. "Tipps" .. R .. "\n" ..
			"- Achte auf genug Tanks für Bosse mit Tankwechsel oder mehreren Zielen.\n" ..
			"- Steht ein Bot falsch, hilft oft " .. C .. "follow" .. R .. " und danach erneut " .. C .. "attack" .. R .. ".\n" ..
			"- Siehe auch die Artikel zu Kampfrausch, Paladin-Auren und Würfeln.",
	},
	{
		id = 5, category = 3, top = true, new = true,
		title = "Boss Rush: Raids ohne Trash",
		keywords = "boss rush trash mobs ohne bossrush schnell",
		text = "Im Boss-Rush-Modus werden Raids (Karazhan bis Sunwell, Molten Core bis Naxxramas) ohne Trash geladen. " ..
			"Bosse, ihre Adds, Event- und Gesprächs-NPCs bleiben erhalten.\n\n" ..
			"Der Modus ist eine Server-Einstellung (" .. C .. "BossRush.Enable" .. R .. " in der mangosd.conf). " ..
			"Er gilt für Instanzen, die nach der Änderung neu erstellt werden - eine bereits laufende ID musst du dafür zurücksetzen.",
	},
	{
		id = 6, category = 3, top = true,
		title = "Kampfrausch / Heldentum steuern",
		keywords = "lust bloodlust heroism kampfrausch heldentum schamane cooldown",
		text = "Schamanen-Bots zünden Kampfrausch bzw. Heldentum im Raid beim Pull - bei manchen Bossen erst später " ..
			"(Lady Vashj unter 49 %, Kael'thas unter 50 %, Illidan unter 30 %, Kil'jaeden unter 25 %).\n\n" ..
			C .. "lust" .. R .. " - ein bereiter Schamane zündet sofort\n" ..
			C .. "lust hold" .. R .. " - aufheben, bis du " .. C .. "lust" .. R .. " schreibst\n" ..
			C .. "lust auto" .. R .. " - wieder selbst nach Boss-Timing\n" ..
			C .. "lust ?" .. R .. " - Modus und Cooldown anzeigen",
	},
	{
		id = 7, category = 3, top = true,
		title = "Paladin-Auren und Segen",
		keywords = "aura auras paladin segen blessing devotion retribution concentration",
		text = C .. "auras" .. R .. " - jeder Paladin nennt seine Aura und seine Segen\n" ..
			C .. "auras auto" .. R .. " - die Paladine jeder Untergruppe verteilen die Auren untereinander\n" ..
			C .. "auras off" .. R .. " - zurück zur Standard-Aura der Skillung\n" ..
			C .. "nc +aura devotion" .. R .. " - feste Aura (auch retribution, concentration, sanctity, shadow, fire, frost, crusader)\n\n" ..
			"Bei Bossen mit Schadensart (z. B. Feuer oder Schatten) übernimmt der erste Paladin der Untergruppe automatisch die passende Widerstandsaura.\n" ..
			"Große Segen wählt der Paladin nach der Rolle, die die meisten Spieler dieser Klasse im Raid haben.",
	},
	{
		id = 8, category = 2, top = true,
		title = "Würfeln, Ausrüstung und Tier-Token",
		keywords = "loot würfeln roll need greed bedarf gier ausrüstung token tier edelstein verzauberung",
		text = H .. "Würfeln" .. R .. "\n" ..
			"Bots würfeln Bedarf erst ab spürbarer Verbesserung. Ist ein echter Spieler in der Gruppe, würfelt nur der Bot mit dem größten Gewinn Bedarf.\n" ..
			C .. "roll feedback" .. R .. " - der Bot begründet seine Würfe im Chat\n\n" ..
			H .. "Ausrüstung" .. R .. "\n" ..
			"Neue Ausrüstung wird beim Anlegen automatisch gesockelt und verzaubert.\n\n" ..
			H .. "Tier-Token (T4-T6)" .. R .. "\n" ..
			"Bots bewerten ein Token wie das passende Set-Teil, tauschen es nach dem Gewinn sofort ein und legen es an. " ..
			"Echte Spieler tauschen Token wie gewohnt beim Händler.",
	},
	{
		id = 9, category = 4, top = true,
		title = "Befehle für Spieler",
		keywords = "befehle commands help server info save passwort password dismount",
		text = C .. ".help" .. R .. " - Hilfe zu einem Befehl\n" ..
			C .. ".commands" .. R .. " - alle Befehle, die du benutzen darfst\n" ..
			C .. ".server info" .. R .. " - Serverversion, Spieler online, Laufzeit\n" ..
			C .. ".server motd" .. R .. " - Nachricht des Tages\n" ..
			C .. ".save" .. R .. " - Charakter speichern\n" ..
			C .. ".dismount" .. R .. " - absitzen\n" ..
			C .. ".start" .. R .. " - Charakter steckt fest: zurück zum Ruhestein-Punkt\n" ..
			C .. ".account password ALT NEU NEU" .. R .. " - Passwort ändern\n" ..
			C .. ".bot ..." .. R .. " - Bot-Befehle (siehe Kategorie Bots)",
	},
	{
		id = 10, category = 5, top = true, hot = true,
		title = "Mein Charakter steckt fest",
		keywords = "stuck fest feststecken hängt teleport unstuck",
		text = "Wenn du irgendwo festhängst:\n\n" ..
			"1. Schreibe " .. C .. ".start" .. R .. " (nicht im Kampf, nicht im Flug). Dein Charakter wirkt den Zauber \"Stuck\" und landet am Ruhestein-Punkt.\n" ..
			"2. Alternativ unten auf " .. H .. "Talk to a GM" .. R .. " / " .. H .. "Character Stuck" .. R .. " klicken.\n" ..
			"3. Hilft beides nicht, erstelle ein GM-Ticket mit Ort und Beschreibung.",
	},
	{
		id = 11, category = 5, top = true,
		title = "GM-Ticket erstellen",
		keywords = "ticket gm hilfe petition support anfrage",
		text = "Über die Knöpfe unten in diesem Fenster (" .. H .. "Talk to a GM" .. R .. " / " .. H .. "Report Issue" .. R ..
			") kannst du ein Ticket an die Spielleiter schicken. Beschreibe kurz, was passiert ist, wo du bist und welcher Charakter betroffen ist.\n\n" ..
			"Dein offenes Ticket kannst du hier jederzeit bearbeiten oder zurückziehen.",
	},
	{
		id = 12, category = 5, top = true,
		title = "Fehler melden und Ideen einreichen",
		keywords = "bug fehler issue github idee vorschlag melden",
		text = "Bug gefunden oder eine Idee? Melde sie als Issue auf\n" ..
			C .. "github.com/puRe991/MangosPro" .. R .. "\n\n" ..
			"Hilfreich sind: was du getan hast, was passiert ist, was du erwartet hast, Boss/Ort und die Klassen der beteiligten Bots.",
	},
};
