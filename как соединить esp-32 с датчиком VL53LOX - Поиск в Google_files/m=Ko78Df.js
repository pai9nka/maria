loaded_h_0(function(_){var window=this;
_.v("Ko78Df");
var EFA=class{constructor(a){this.state=a}getId(){return this.state.id}getPushId(){return this.state.pushId}getUrl(){return this.state.url}getUserData(){return this.state.userData}};
_.Zf(_.S2a,class extends _.ho{static Ta(){return{service:{history:_.Bgb}}}constructor(a){super();this.history=a.service.history;this.yD=document.body;this.oa=new Map;this.history.addListener((b,c,d)=>{if(d.y6)for(const e of d.y6)if(this.oa.has(e.id)){const f=this.oa.get(e.id);f&&_.oe(()=>{f(e.Fya)});this.oa.delete(e.id)}d.userInitiated&&this.yD.dispatchEvent(new CustomEvent("FWkcec"))})}pushAsync(a,b,c,d){a=d?this.history.oa(a,b):this.history.pushAsync(a,b);return Promise.resolve(a.then(e=>{c&&this.oa.set(e.id,
c);return e.id}))}pop(a,b){a=b?this.history.Ca(a):this.history.pop(a);return Promise.resolve(a.then(c=>c?new EFA(c):null))}getState(){const a=this.history.getState();return a?new EFA(a):null}getCurrentUrl(){return this.history.getCurrentUrl()}getEventTarget(){return this.yD}navigate(a,b){a=this.history.navigate(a,b).committed;return Promise.resolve(a.then(c=>new EFA(c)))}});
_.x();
});
// Google Inc.
