// Optional end-to-end checks. Build with SLINGSHIFT_TEST_HOOKS=ON; release omits all QA hooks.
// npm install --no-save playwright && npx playwright install chromium
const http=require('http'),fs=require('fs'),path=require('path'),assert=require('assert');
const {chromium}=require(process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES?process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES+'/playwright':'playwright');
const project=path.resolve(__dirname,'..');
const root=path.resolve(project,process.argv[2]||'build/web-qa');
const out=path.resolve(project,process.env.SLINGSHIFT_QA_OUTPUT||'build/browser-qa');
fs.mkdirSync(out,{recursive:true});
let checks=0;
const report={browser:'',checks:[],routes:[],errors:[]};
function check(condition,message){assert(condition,message);++checks;report.checks.push(message);console.log('PASS',message);}
const server=http.createServer((req,res)=>{
  const name=decodeURIComponent(req.url.split('?')[0]);
  const file=path.resolve(root,'.'+(name==='/'?'/index.html':name));
  if(!file.startsWith(root+path.sep)){res.writeHead(403);res.end();return;}
  try{res.setHeader('Content-Type',file.endsWith('.wasm')?'application/wasm':file.endsWith('.js')?'application/javascript':file.endsWith('.html')?'text/html':'application/octet-stream');res.end(fs.readFileSync(file));}
  catch(e){res.writeHead(404);res.end('Not found');}
});
(async()=>{
  await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
  const browser=await chromium.launch({headless:true,executablePath:process.env.SLINGSHIFT_BROWSER||undefined,args:['--no-sandbox','--disable-dev-shm-usage','--use-gl=angle','--use-angle=swiftshader','--enable-unsafe-swiftshader','--disable-gpu-sandbox','--no-zygote','--single-process']});
  report.browser=browser.version();
  const url='http://127.0.0.1:'+server.address().port;
  const state=page=>page.evaluate(()=>window.SlingQA.state());
  const wait=async(page,fn,timeout=14000)=>page.waitForFunction(fn,null,{timeout});
  const click=async(page,x,y)=>{await page.mouse.click(x,y,{delay:85});await page.waitForTimeout(100);};
  const key=async(page,k)=>{await page.keyboard.press(k,{delay:85});await page.waitForTimeout(80);};
  const boot=async(page)=>{
    page.on('pageerror',e=>report.errors.push(e.message));
    await page.goto(url);await wait(page,()=>!!window.SlingQA,30000);await page.waitForTimeout(350);
  };
  const drag=async(page,start,end)=>{
    await page.mouse.move(start.x,start.y);await page.mouse.down();await page.waitForTimeout(60);
    await page.mouse.move(end.x,end.y,{steps:12});await page.waitForTimeout(100);await page.mouse.up();await page.waitForTimeout(90);
  };
  try{
    const context=await browser.newContext({viewport:{width:1280,height:800}});
    const page=await context.newPage();await boot(page);
    check((await state(page)).screen===0,'home screen boots');
    await page.screenshot({path:path.join(out,'home.png')});
    await key(page,'Tab');await key(page,'Enter');
    check((await state(page)).screen===2,'keyboard navigation starts campaign');
    for(let i=0;i<12;++i){
      let s=await state(page);check(s.level===i && s.mode===0,'signal '+(i+1)+' enters aiming state');
      if(i===0) await drag(page,{x:s.startX,y:s.startY},{x:s.pullX,y:s.pullY});
      else {
        await drag(page,{x:600,y:440},{x:600+(s.aimX-s.solutionX)/.45,y:440+(s.aimY-s.solutionY)/.45});
        check((await state(page)).mode===0,'signal '+(i+1)+' refinement keeps launch staged');
        await key(page,'Space');
      }
      check((await state(page)).mode===1,'signal '+(i+1)+' launches through mouse input');
      if(i===0){
        await click(page,1190,40);s=await state(page);check(s.paused===1,'pause button holds flight');
        const t=s.time;await page.waitForTimeout(250);check((await state(page)).time===t,'paused physics clock is frozen');
        await key(page,'Escape');check((await state(page)).paused===0,'Escape resumes flight');
      }
      await wait(page,()=>[2,3].includes(window.SlingQA.state().mode));
      s=await state(page);check(s.mode===2 && s.outcome===1,'signal '+(i+1)+' arrives through actual game loop');
      check(s.stars===(i+1)*3,'signal '+(i+1)+' earns three stars and saves progression');
      report.routes.push({level:i+1,time:s.time});
      await page.waitForTimeout(1050);
      if(i===3 || i===6 || i===11) await page.screenshot({path:path.join(out,'arrival-'+(i+1)+'.png')});
      await click(page,640,474);
    }
    check((await state(page)).screen===5,'final gate leads to campaign ending');
    await page.screenshot({path:path.join(out,'complete.png')});
    await page.reload();await wait(page,()=>!!window.SlingQA);await page.waitForTimeout(250);
    check((await state(page)).stars===36 && (await state(page)).unlocked===11,'progress survives browser reload');
    await click(page,154,522);check((await state(page)).screen===3,'settings menu opens');
    await click(page,870,195); // Decrease volume.
    const save=await page.evaluate(()=>localStorage.getItem('slingshift.save.v1'));
    check(!!save && save.includes('SLINGSHIFT 1'),'preferences write a versioned local save');
    check(parseFloat(save.trim().split('\n').at(-1))>.7,'volume control changes saved level');
    await page.screenshot({path:path.join(out,'settings.png')});
    await key(page,'Escape');
    await page.evaluate(()=>window.SlingQA.level(3));await page.waitForTimeout(100);
    let s=await state(page);
    await drag(page,{x:s.startX,y:s.startY},{x:s.startX-300/2.15*s.pixelScale,y:s.startY});
    await wait(page,()=>window.SlingQA.state().mode===3);
    check((await state(page)).outcome===2,'star collision enters failure state');
    await page.waitForTimeout(600);await page.screenshot({path:path.join(out,'failure.png')});
    await key(page,'r');check((await state(page)).mode===0 && (await state(page)).time===0,'R instantly retries after failure');
    await page.evaluate(()=>window.SlingQA.level(0));await page.waitForTimeout(100);s=await state(page);
    await drag(page,{x:s.startX,y:s.startY},{x:s.pullX,y:s.pullY});await page.waitForTimeout(170);
    await page.mouse.move(550,500);await page.mouse.down();await page.waitForTimeout(80);await page.mouse.move(560,475);
    const before=(await state(page)).time;await page.waitForTimeout(400);const slow=await state(page);
    check(slow.dragging===1 && slow.time-before<.12,'midflight drag dilates time while aiming');
    await page.mouse.up();await page.waitForTimeout(100);check((await state(page)).corrected===1,'release spends the correction burn');
    await page.mouse.move(600,500);await page.mouse.down();await page.waitForTimeout(90);
    check((await state(page)).dragging===0,'spent burn cannot be used twice');await page.mouse.up();
    await key(page,'r');await page.evaluate(()=>window.SlingQA.level(11));await page.waitForTimeout(150);
    await click(page,117,778); // Route hint.
    await click(page,270,778); // Gravity field.
    await page.screenshot({path:path.join(out,'deep-field.png')});

    const mobile=await browser.newContext({viewport:{width:390,height:844},isMobile:true,hasTouch:true,deviceScaleFactor:1});
    await mobile.addInitScript(()=>localStorage.removeItem('slingshift.save.v1'));
    const phone=await mobile.newPage();await boot(phone);
    const client=await mobile.newCDPSession(phone);
    const touch=async(type,p)=>client.send('Input.dispatchTouchEvent',{type,touchPoints:p?[{x:p.x,y:p.y}]:[]});
    await phone.touchscreen.tap(195,598);await phone.waitForTimeout(200);
    check((await state(phone)).screen===2,'portrait touch menu starts game');
    check((await state(phone)).level===0,'fresh mobile profile starts at the first signal');
    s=await state(phone);
    await touch('touchStart',{x:s.startX,y:s.startY});await phone.waitForTimeout(80);
    for(let n=1;n<=10;++n){await touch('touchMove',{x:s.startX+(s.pullX-s.startX)*n/10,y:s.startY+(s.pullY-s.startY)*n/10});await phone.waitForTimeout(16);}
    await phone.waitForTimeout(100);await phone.screenshot({path:path.join(out,'mobile-aim.png')});
    await touch('touchEnd');await phone.waitForTimeout(100);
    check((await state(phone)).mode===1,'touch slingshot launches craft');
    await wait(phone,()=>[2,3].includes(window.SlingQA.state().mode));
    check((await state(phone)).mode===2,'touch route reaches destination');
    await phone.waitForTimeout(1050);await phone.screenshot({path:path.join(out,'mobile-arrival.png')});
    await phone.evaluate(()=>window.SlingQA.level(11));await phone.waitForTimeout(100);s=await state(phone);
    const scale=390/480,anchor={x:210,y:300};
    const refined={x:anchor.x+(s.aimX-s.solutionX)/.45*scale,y:anchor.y+(s.aimY-s.solutionY)/.45*scale};
    await touch('touchStart',anchor);await phone.waitForTimeout(70);
    for(let n=1;n<=12;++n){await touch('touchMove',{x:anchor.x+(refined.x-anchor.x)*n/12,y:anchor.y+(refined.y-anchor.y)*n/12});await phone.waitForTimeout(16);}
    await touch('touchEnd');await phone.waitForTimeout(100);
    check((await state(phone)).mode===0,'touch refinement holds a staged route');
    await phone.touchscreen.tap(195,473);await phone.waitForTimeout(100);
    check((await state(phone)).mode===1,'touch Launch button starts refined route');
    await wait(phone,()=>[2,3].includes(window.SlingQA.state().mode));
    check((await state(phone)).mode===2,'final signal is achievable through touch refinement');
    await phone.waitForTimeout(1050);await phone.screenshot({path:path.join(out,'mobile-finale.png')});
    await phone.evaluate(()=>window.SlingQA.level(0));await phone.waitForTimeout(100);s=await state(phone);
    await touch('touchStart',{x:s.startX,y:s.startY});await phone.waitForTimeout(80);
    await touch('touchMove',{x:s.startX-10,y:s.startY+10});await phone.waitForTimeout(80);
    await touch('touchCancel');await phone.waitForTimeout(100);
    check((await state(phone)).mode===0 && !(await state(phone)).dragging,'cancelled touch never launches or spends a burn');
    await phone.setViewportSize({width:844,height:390});await phone.waitForTimeout(200);
    await phone.evaluate(()=>window.SlingQA.level(11));await phone.waitForTimeout(100);
    await phone.screenshot({path:path.join(out,'mobile-landscape.png')});
    check((await state(phone)).screen===2,'game survives mobile orientation change');
    check(report.errors.length===0,'no uncaught JavaScript runtime errors');
    report.totalChecks=checks;console.log('Completed',checks,'browser checks.');
  }finally{
    fs.writeFileSync(path.join(out,'results.json'),JSON.stringify(report,null,2));
    await browser.close();server.close();
  }
})().catch(e=>{console.error(e);server.close();process.exitCode=1;});
