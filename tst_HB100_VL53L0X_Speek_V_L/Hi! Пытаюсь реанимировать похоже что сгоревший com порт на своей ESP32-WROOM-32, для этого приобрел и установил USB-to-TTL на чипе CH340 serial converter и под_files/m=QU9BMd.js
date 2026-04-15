loaded_h_0(function(_){var window=this;
_.v("QU9BMd");
var eSA=class{constructor(a){this.state=a}getId(){return this.state.id}getPushId(){return this.state.pushId}getUrl(){return this.state.url}getUserData(){return this.state.userData}};
_.$f(_.i3a,class extends _.ko{static Ta(){return{service:{history:_.Igb}}}constructor(a){super();this.history=a.service.history;this.zD=document.body;this.oa=new Map;this.history.addListener((b,c,d)=>{if(d.x6)for(const e of d.x6)if(this.oa.has(e.id)){const f=this.oa.get(e.id);f&&_.pe(()=>{f(e.Cya)});this.oa.delete(e.id)}d.userInitiated&&this.zD.dispatchEvent(new CustomEvent("FWkcec"))})}pushAsync(a,b,c,d){a=d?this.history.oa(a,b):this.history.pushAsync(a,b);return Promise.resolve(a.then(e=>{c&&this.oa.set(e.id,
c);return e.id}))}pop(a,b){a=b?this.history.Ca(a):this.history.pop(a);return Promise.resolve(a.then(c=>c?new eSA(c):null))}getState(){const a=this.history.getState();return a?new eSA(a):null}getCurrentUrl(){return this.history.getCurrentUrl()}getEventTarget(){return this.zD}navigate(a,b){a=this.history.navigate(a,b).committed;return Promise.resolve(a.then(c=>new eSA(c)))}});
_.x();
});
// Google Inc.
