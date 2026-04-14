loaded_h_0(function(_){var window=this;
_.v("QU9BMd");
var $fB=class{constructor(a){this.state=a}getId(){return this.state.id}getPushId(){return this.state.pushId}getUrl(){return this.state.url}getUserData(){return this.state.userData}};
_.Zf(_.P2a,class extends _.jo{static Ta(){return{service:{history:_.ygb}}}constructor(a){super();this.history=a.service.history;this.bG=document.body;this.oa=new Map;this.history.addListener((b,c,d)=>{if(d.eba)for(const e of d.eba)if(this.oa.has(e.id)){const f=this.oa.get(e.id);f&&_.qe(()=>{f(e.RFa)});this.oa.delete(e.id)}d.userInitiated&&this.bG.dispatchEvent(new CustomEvent("FWkcec"))})}pushAsync(a,b,c,d){a=d?this.history.Aa(a,b):this.history.pushAsync(a,b);return Promise.resolve(a.then(e=>{c&&
this.oa.set(e.id,c);return e.id}))}pop(a,b){a=b?this.history.Ca(a):this.history.pop(a);return Promise.resolve(a.then(c=>c?new $fB(c):null))}getState(){const a=this.history.getState();return a?new $fB(a):null}getCurrentUrl(){return this.history.getCurrentUrl()}getEventTarget(){return this.bG}navigate(a,b){a=this.history.navigate(a,b).committed;return Promise.resolve(a.then(c=>new $fB(c)))}});
_.x();
});
// Google Inc.
