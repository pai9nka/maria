loaded_h_0(function(_){var window=this;
_.v("QU9BMd");
var NdB=class{constructor(a){this.state=a}getId(){return this.state.id}getPushId(){return this.state.pushId}getUrl(){return this.state.url}getUserData(){return this.state.userData}};
_.Xf(_.V2a,class extends _.jo{static Sa(){return{service:{history:_.Dgb}}}constructor(a){super();this.history=a.service.history;this.UF=document.body;this.oa=new Map;this.history.addListener((b,c,d)=>{if(d.Taa)for(const e of d.Taa)if(this.oa.has(e.id)){const f=this.oa.get(e.id);f&&_.oe(()=>{f(e.DFa)});this.oa.delete(e.id)}d.userInitiated&&this.UF.dispatchEvent(new CustomEvent("FWkcec"))})}pushAsync(a,b,c,d){a=d?this.history.Aa(a,b):this.history.pushAsync(a,b);return Promise.resolve(a.then(e=>{c&&
this.oa.set(e.id,c);return e.id}))}pop(a,b){a=b?this.history.Ca(a):this.history.pop(a);return Promise.resolve(a.then(c=>c?new NdB(c):null))}getState(){const a=this.history.getState();return a?new NdB(a):null}getCurrentUrl(){return this.history.getCurrentUrl()}getEventTarget(){return this.UF}navigate(a,b){a=this.history.navigate(a,b).committed;return Promise.resolve(a.then(c=>new NdB(c)))}});
_.x();
});
// Google Inc.
