loaded_h_0(function(_){var window=this;
_.v("QU9BMd");
var $IA=class{constructor(a){this.state=a}getId(){return this.state.id}getPushId(){return this.state.pushId}getUrl(){return this.state.url}getUserData(){return this.state.userData}};
_.Yf(_.$2a,class extends _.fo{static Ta(){return{service:{history:_.Dgb}}}constructor(a){super();this.history=a.service.history;this.qD=document.body;this.oa=new Map;this.history.addListener((b,c,d)=>{if(d.e6)for(const e of d.e6)if(this.oa.has(e.id)){const f=this.oa.get(e.id);f&&_.pe(()=>{f(e.Aya)});this.oa.delete(e.id)}d.userInitiated&&this.qD.dispatchEvent(new CustomEvent("FWkcec"))})}pushAsync(a,b,c,d){a=d?this.history.oa(a,b):this.history.pushAsync(a,b);return Promise.resolve(a.then(e=>{c&&this.oa.set(e.id,
c);return e.id}))}pop(a,b){a=b?this.history.Ca(a):this.history.pop(a);return Promise.resolve(a.then(c=>c?new $IA(c):null))}getState(){const a=this.history.getState();return a?new $IA(a):null}getCurrentUrl(){return this.history.getCurrentUrl()}getEventTarget(){return this.qD}navigate(a,b){a=this.history.navigate(a,b).committed;return Promise.resolve(a.then(c=>new $IA(c)))}});
_.x();
});
// Google Inc.
