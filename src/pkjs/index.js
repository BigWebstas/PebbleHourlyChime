/*
 * Phone-side glue for Hourly Chime.
 *
 * Presents a self-contained config page (no external hosting, no Clay
 * dependency) as a data: URL, then forwards the chosen values to the watch
 * as an AppMessage using the CFG_* message keys.
 */

var DEFAULTS = {
  CFG_ENABLED: 1,
  CFG_MODE: 0,          // 0 = wakeup, 1 = background worker
  CFG_SOUND: 1,
  CFG_VIBE: 1,
  CFG_VOLUME: 70,
  CFG_START_HOUR: 8,
  CFG_END_HOUR: 22,
  CFG_STYLE: 1,         // 0 beep, 1 westminster, 2 cuckoo, 3 strike, 4 mario, 5 starwars, 6 spongebob, 7 xfiles, 8 batman, 9 casio, 10 nokia
  CFG_STRIKE_COUNT: 0   // 0 = strike the current hour
};

function loadConfig() {
  var cfg = JSON.parse(JSON.stringify(DEFAULTS));
  try {
    var raw = localStorage.getItem('hc_config');
    if (raw) {
      var saved = JSON.parse(raw);
      for (var k in DEFAULTS) {
        if (saved.hasOwnProperty(k)) {
          cfg[k] = saved[k];
        }
      }
    }
  } catch (e) {
    console.log('loadConfig failed: ' + e);
  }
  return cfg;
}

function saveConfig(cfg) {
  try {
    localStorage.setItem('hc_config', JSON.stringify(cfg));
  } catch (e) {
    console.log('saveConfig failed: ' + e);
  }
}

function hourOptions(selected) {
  var out = '';
  for (var h = 0; h < 24; h++) {
    var label = (h < 10 ? '0' + h : '' + h) + ':00';
    out += '<option value="' + h + '"' + (h === selected ? ' selected' : '') + '>' + label + '</option>';
  }
  return out;
}

function buildPage(cfg) {
  function sel(v, want) { return v === want ? ' selected' : ''; }
  function chk(v) { return v ? ' checked' : ''; }

  return '<!DOCTYPE html><html><head><meta charset="utf-8">' +
    '<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">' +
    '<meta name="color-scheme" content="light dark">' +
    '<title>Hourly Chime</title><style>' +
    ':root{--bg:#efeff4;--grp:#fff;--border:#d1d1d6;--fg:#1c1c1e;--hint:#6c6c70;' +
    '--field-bg:#e9e9eb;--field-fg:#1c1c1e;--accent:#007aff}' +
    '@media (prefers-color-scheme:dark){:root{--bg:#1c1c1e;--grp:#2c2c2e;--border:#3a3a3c;' +
    '--fg:#f2f2f7;--hint:#8e8e93;--field-bg:#3a3a3c;--field-fg:#fff;--accent:#0a84ff}}' +
    'body{font-family:-apple-system,Roboto,Helvetica,sans-serif;margin:0;background:var(--bg);color:var(--fg)}' +
    'h1{font-size:19px;font-weight:600;padding:18px 16px 4px}' +
    '.grp{background:var(--grp);margin:12px 0;border-top:1px solid var(--border);border-bottom:1px solid var(--border)}' +
    '.row{display:flex;align-items:center;justify-content:space-between;padding:12px 16px;border-bottom:1px solid var(--border)}' +
    '.row:last-child{border-bottom:0}' +
    '.row label{flex:1;font-size:16px}' +
    'select,input[type=range]{font-size:16px}' +
    'select{background:var(--field-bg);color:var(--field-fg);border:0;border-radius:8px;padding:6px 8px}' +
    'input[type=checkbox]{width:22px;height:22px}' +
    '.hint{font-size:12px;color:var(--hint);padding:6px 16px 14px}' +
    'button{width:calc(100% - 32px);margin:20px 16px 40px;padding:14px;font-size:17px;font-weight:600;' +
    'border:0;border-radius:12px;background:var(--accent);color:#fff}' +
    '#vol{width:150px}' +
    '</style></head><body>' +

    '<h1>Hourly Chime</h1>' +

    '<div class="grp">' +
      '<div class="row"><label for="enabled">Enabled</label>' +
        '<input type="checkbox" id="enabled"' + chk(cfg.CFG_ENABLED) + '></div>' +
      '<div class="row"><label for="mode">Trigger</label>' +
        '<select id="mode">' +
          '<option value="0"' + sel(cfg.CFG_MODE, 0) + '>Wakeup (recommended)</option>' +
          '<option value="1"' + sel(cfg.CFG_MODE, 1) + '>Background worker</option>' +
        '</select></div>' +
    '</div>' +
    '<div class="hint">Wakeup relaunches the app each hour and uses no battery between chimes. ' +
      'Background worker stays resident — but the watch allows only one worker app at a time ' +
      '(it will replace a step/sleep tracker’s worker).</div>' +

    '<div class="grp">' +
      '<div class="row"><label for="sound">Speaker</label>' +
        '<input type="checkbox" id="sound"' + chk(cfg.CFG_SOUND) + '></div>' +
      '<div class="row"><label for="vibe">Vibration</label>' +
        '<input type="checkbox" id="vibe"' + chk(cfg.CFG_VIBE) + '></div>' +
      '<div class="row"><label for="vol">Volume</label>' +
        '<input type="range" id="vol" min="0" max="100" step="5" value="' + cfg.CFG_VOLUME + '"></div>' +
    '</div>' +
    '<div class="hint">Speaker requires a Pebble Time 2 or Pebble 2 Duo, and is silenced by Quiet Time / system mute.</div>' +

    '<div class="grp">' +
      '<div class="row"><label for="style">Sound</label>' +
        '<select id="style">' +
          '<option value="0"' + sel(cfg.CFG_STYLE, 0) + '>Single beep</option>' +
          '<option value="1"' + sel(cfg.CFG_STYLE, 1) + '>Westminster</option>' +
          '<option value="2"' + sel(cfg.CFG_STYLE, 2) + '>Cuckoo</option>' +
          '<option value="3"' + sel(cfg.CFG_STYLE, 3) + '>Hour strikes</option>' +
          '<option value="4"' + sel(cfg.CFG_STYLE, 4) + '>Mario</option>' +
          '<option value="5"' + sel(cfg.CFG_STYLE, 5) + '>Star Wars</option>' +
          '<option value="6"' + sel(cfg.CFG_STYLE, 6) + '>SpongeBob</option>' +
          '<option value="7"' + sel(cfg.CFG_STYLE, 7) + '>X-Files</option>' +
          '<option value="8"' + sel(cfg.CFG_STYLE, 8) + '>Batman</option>' +
          '<option value="9"' + sel(cfg.CFG_STYLE, 9) + '>Casio beep</option>' +
          '<option value="10"' + sel(cfg.CFG_STYLE, 10) + '>Nokia tune</option>' +
        '</select></div>' +
      '<div class="row"><label for="strike">Strike count</label>' +
        '<select id="strike">' +
          '<option value="0"' + sel(cfg.CFG_STRIKE_COUNT, 0) + '>Current hour (1–12)</option>' +
          (function () {
            var o = '';
            for (var i = 1; i <= 12; i++) {
              o += '<option value="' + i + '"' + sel(cfg.CFG_STRIKE_COUNT, i) + '>' + i + '</option>';
            }
            return o;
          })() +
        '</select></div>' +
    '</div>' +
    '<div class="hint">Strike count applies to the “Hour strikes” sound and its matching vibration.</div>' +

    '<div class="grp">' +
      '<div class="row"><label for="start">Active from</label>' +
        '<select id="start">' + hourOptions(cfg.CFG_START_HOUR) + '</select></div>' +
      '<div class="row"><label for="end">Active until</label>' +
        '<select id="end">' + hourOptions(cfg.CFG_END_HOUR) + '</select></div>' +
    '</div>' +
    '<div class="hint">Set “from” later than “until” to span midnight (e.g. 22:00 → 06:00). ' +
      'Equal values chime every hour, all day.</div>' +

    '<button id="save">Save</button>' +

    '<script>' +
    'function gi(id){return document.getElementById(id);}' +
    'gi("save").addEventListener("click",function(){' +
      'var out={' +
        'CFG_ENABLED:gi("enabled").checked?1:0,' +
        'CFG_MODE:parseInt(gi("mode").value,10),' +
        'CFG_SOUND:gi("sound").checked?1:0,' +
        'CFG_VIBE:gi("vibe").checked?1:0,' +
        'CFG_VOLUME:parseInt(gi("vol").value,10),' +
        'CFG_START_HOUR:parseInt(gi("start").value,10),' +
        'CFG_END_HOUR:parseInt(gi("end").value,10),' +
        'CFG_STYLE:parseInt(gi("style").value,10),' +
        'CFG_STRIKE_COUNT:parseInt(gi("strike").value,10)' +
      '};' +
      'document.location="pebblejs://close#"+encodeURIComponent(JSON.stringify(out));' +
    '});' +
    '</script></body></html>';
}

Pebble.addEventListener('ready', function () {
  console.log('Hourly Chime PebbleKit JS ready');
});

Pebble.addEventListener('showConfiguration', function () {
  var cfg = loadConfig();
  Pebble.openURL('data:text/html,' + encodeURIComponent(buildPage(cfg)));
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (!e || !e.response) {
    return;  // user backed out
  }

  var cfg;
  try {
    cfg = JSON.parse(decodeURIComponent(e.response));
  } catch (err) {
    try {
      cfg = JSON.parse(e.response);
    } catch (err2) {
      console.log('could not parse config response');
      return;
    }
  }

  saveConfig(cfg);
  Pebble.sendAppMessage(cfg,
    function () { console.log('config delivered to watch'); },
    function (err) { console.log('config send failed: ' + JSON.stringify(err)); });
});
