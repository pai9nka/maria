loaded_h_0(function(_){var window=this;
_.gKq=_.y("JjeCTe",[_.s1a]);
_.YJq=_.y("TPfxRd",[_.s1a]);
_.v("yxCHBd");
var Xps=function(a){if(a!==void 0)return{isExpanded:a}},Yps=class{constructor(a){this.oa=a}subscribeToExpanderState(a){this.oa.isExpanded.subscribe((b,c)=>{a(Xps(b),Xps(c))})}},Zps=class{constructor(a){this.oa=a}subscribeToInputPlateState(a){this.oa.hasSubmittedQuery.subscribe((b,c)=>{a({hasSubmittedQuery:b},c!==void 0?{hasSubmittedQuery:c}:void 0)})}setHasSubmittedQuery(a){this.oa.hasSubmittedQuery.value=a}subscribeToOverlayState(a){this.oa.oNa.subscribe((b,c)=>{a(b,c)})}setOverlayState(a){this.oa.oNa.value=
a}},$ps=function(a,b,c){(b=b.closest(`[jsmodel~='${c}']`))||(b=document.querySelector(`[jsmodel~='${c}']`));if(!b)return null;let d;return((d=a.oa.get(c))==null?void 0:d.get(b))||null};_.Yf(_.t1a,class extends _.fo{constructor(){super();this.oa=new Map}Oxc(a,b){this.oa.has(a)||this.oa.set(a,new Map);this.oa.get(a).set(b.Xca(),b)}getExpanderModel(a){return(a=$ps(this,a,_.YJq))?new Yps(a):null}getAimOverlayModel(a){return(a=$ps(this,a,_.gKq))?new Zps(a):null}});
_.x();
_.v("QE20Be");
_.afr=new _.Tf(_.s1a);
_.x();
_.v("a7QTqd");
_.Yf(_.d3a,class extends _.fo{static Ta(){return{service:{FWa:_.afr}}}constructor(a){super();this.FWa=a.service.FWa}getExpanderModel(a){return this.FWa.getExpanderModel(a)}getAimOverlayModel(a){return this.FWa.getAimOverlayModel(a)}});
_.x();
});
// Google Inc.
