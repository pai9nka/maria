loaded_h_0(function(_){var window=this;
_.CMq=_.y("vL8wte",[_.j1a]);
_.tLq=_.y("JjeCTe",[_.j1a]);
_.rLq=_.y("TPfxRd",[_.j1a]);
_.v("yxCHBd");
var mss=function(a){if(a!==void 0)return{isExpanded:a}},nss=class{constructor(a){this.oa=a}subscribeToExpanderState(a){this.oa.isExpanded.subscribe((b,c)=>{a(mss(b),mss(c))})}},oss=class{constructor(a){this.oa=a}subscribeToInputPlateState(a){this.oa.hasSubmittedQuery.subscribe((b,c)=>{a({hasSubmittedQuery:b},c!==void 0?{hasSubmittedQuery:c}:void 0)})}setHasSubmittedQuery(a){this.oa.hasSubmittedQuery.value=a}subscribeToOverlayState(a){this.oa.RNa.subscribe((b,c)=>{a(b,c)})}setOverlayState(a){this.oa.RNa.value=
a}},pss=class{constructor(a){this.oa=a}subscribeToNonPersonalizedQuerySubmission(a){this.oa.Lhe(a)}},qss=function(a,b,c){(b=b.closest(`[jsmodel~='${c}']`))||(b=document.querySelector(`[jsmodel~='${c}']`));if(!b)return null;let d;return((d=a.oa.get(c))==null?void 0:d.get(b))||null};
_.Zf(_.k1a,class extends _.jo{constructor(){super();this.oa=new Map}z1b(a,b){this.oa.has(a)||this.oa.set(a,new Map);this.oa.get(a).set(b.pda(),b)}getExpanderModel(a){return(a=qss(this,a,_.rLq))?new nss(a):null}getAimOverlayModel(a){return(a=qss(this,a,_.tLq))?new oss(a):null}getPersonalizationModel(a){return(a=qss(this,a,_.CMq))?new pss(a):null}});
_.x();
_.v("QE20Be");
_.Jfr=new _.Uf(_.j1a);
_.x();
_.v("a7QTqd");
_.Zf(_.T2a,class extends _.jo{static Ta(){return{service:{fya:_.Jfr}}}constructor(a){super();this.fya=a.service.fya}getExpanderModel(a){return this.fya.getExpanderModel(a)}getAimOverlayModel(a){return this.fya.getAimOverlayModel(a)}getPersonalizationModel(a){return this.fya.getPersonalizationModel(a)}});
_.x();
});
// Google Inc.
